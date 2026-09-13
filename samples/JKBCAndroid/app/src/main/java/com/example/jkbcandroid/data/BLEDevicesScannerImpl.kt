package com.example.jkbcandroid.data

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Build
import android.util.Log
import com.example.jkbcandroid.domain.BLEDevice
import com.example.jkbcandroid.domain.BLEDevicesScanner
import com.example.jkbcandroid.domain.BleAddress
import com.example.jkbcandroid.domain.ScanResult
import com.example.jkbcandroid.domain.SessionId
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withContext
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicInteger
import javax.inject.Inject
import javax.inject.Singleton
import kotlin.coroutines.resume

@Singleton
class BLEDevicesScannerImpl @Inject constructor(
    @ApplicationContext private val context: Context
) : BLEDevicesScanner {

    private val resolverScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    private val bluetoothManager by lazy {
        context.getSystemService(BluetoothManager::class.java)
    }

    private val bluetoothLeScanner by lazy {
        bluetoothManager?.adapter?.bluetoothLeScanner
    }

    private val scanResultFlow = MutableStateFlow(
        ScanResult(
            devices = emptyList(),
            timestamp = System.currentTimeMillis(),
            error = null
        )
    )

    private val sessions = ConcurrentHashMap<SessionId, Unit>()
    private val sessionIdCounter = AtomicInteger(0)
    private val resolvedNamesCache = ConcurrentHashMap<BleAddress, String>()
    private val resolveQueue = LinkedBlockingQueue<BleAddress>()
    private val queuedAddresses = ConcurrentHashMap.newKeySet<BleAddress>()
    private val addressesBeingResolved = ConcurrentHashMap.newKeySet<BleAddress>()

    init {
        resolverScope.launch {
            processResolveQueue()
        }
    }

    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: android.bluetooth.le.ScanResult) {
            val address = result.device.address
            val resolvedName = resolveDisplayedName(result)
            val device = BLEDevice(
                name = resolvedName,
                address = address,
                rssi = result.rssi
            )

            scanResultFlow.update { current ->
                val devicesByAddress = current.devices.associateBy { it.address }.toMutableMap()
                devicesByAddress[address] = device
                current.copy(
                    devices = devicesByAddress.values.toList(),
                    timestamp = System.currentTimeMillis(),
                    error = null
                )
            }
        }

        override fun onScanFailed(errorCode: Int) {
            scanResultFlow.update { current ->
                current.copy(
                    timestamp = System.currentTimeMillis(),
                    error = IllegalStateException("BLE scan failed with error code: $errorCode")
                )
            }
        }
    }

    @SuppressLint("MissingPermission")
    override fun startScanSession(): SessionId {
        val id = sessionIdCounter.incrementAndGet()
        val wasEmpty = sessions.isEmpty()
        sessions[id] = Unit
        if (wasEmpty) {
            scanResultFlow.value = ScanResult(
                devices = emptyList(),
                timestamp = System.currentTimeMillis(),
                error = null
            )
            try {
                val scanSettings = ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                    .setMatchMode(ScanSettings.MATCH_MODE_AGGRESSIVE)
                    .setNumOfMatches(ScanSettings.MATCH_NUM_ONE_ADVERTISEMENT)
                    .setReportDelay(0L)
                    .build()

                bluetoothLeScanner?.startScan(null, scanSettings, scanCallback)
            } catch (e: SecurityException) {
                scanResultFlow.value = ScanResult(
                    devices = emptyList(),
                    timestamp = System.currentTimeMillis(),
                    error = e
                )
            }
        }
        return id
    }

    @SuppressLint("MissingPermission")
    override fun stopScanSession(sessionId: SessionId): Boolean {
        val removed = sessions.remove(sessionId) != null
        if (removed && sessions.isEmpty()) {
            bluetoothLeScanner?.stopScan(scanCallback)
        }
        return removed
    }

    override fun getScannedDevices(sessionId: SessionId): ScanResult {
        if (!sessions.containsKey(sessionId)) {
            return ScanResult(
                devices = emptyList(),
                timestamp = System.currentTimeMillis(),
                error = IllegalArgumentException("Session $sessionId is not active")
            )
        }
        return scanResultFlow.value
    }

    override fun observeScannedDevices(sessionId: SessionId): Flow<ScanResult> {
        return scanResultFlow
    }

    private fun resolveDisplayedName(result: android.bluetooth.le.ScanResult): String {
        val address = result.device.address
        val advertisedName = result.scanRecord?.deviceName
        if (!advertisedName.isNullOrBlank()) {
            resolvedNamesCache[address] = advertisedName
            return advertisedName
        }

        val deviceName = result.device.name
        if (!deviceName.isNullOrBlank()) {
            resolvedNamesCache[address] = deviceName
            return deviceName
        }

        val cachedName = resolvedNamesCache[address]
        if (!cachedName.isNullOrBlank()) {
            return cachedName
        }

        enqueueNameResolution(address)
        return ""
    }

    private fun enqueueNameResolution(address: BleAddress) {
        if (queuedAddresses.add(address)) {
            resolveQueue.offer(address)
        }
    }

    private suspend fun processResolveQueue() {
        while (true) {
            val address = resolveQueue.poll(1, TimeUnit.SECONDS) ?: continue
            queuedAddresses.remove(address)
            Log.d("BLEDevicesScannerImpl", "Resolving name for $address")

            if (!addressesBeingResolved.add(address)) {
                continue
            }

            try {
                val resolvedName = resolveNameViaGatt(address)
                Log.d("BLEDevicesScannerImpl", "Resolved name for $address: $resolvedName")
                if (!resolvedName.isNullOrBlank()) {
                    resolvedNamesCache[address] = resolvedName
                    updateResolvedDeviceName(address, resolvedName)
                }
            } catch (securityException: SecurityException) {
                Log.e("BLEDevicesScannerImpl", "Missing Bluetooth permission while resolving $address", securityException)
            } catch (exception: Exception) {
                Log.e("BLEDevicesScannerImpl", "Failed to resolve name for $address", exception)
            } finally {
                addressesBeingResolved.remove(address)
            }
        }
    }

    private suspend fun resolveNameViaGatt(address: BleAddress): String? = withContext(Dispatchers.IO) {
        val adapter = bluetoothManager?.adapter ?: return@withContext null
        val remoteDevice = try {
            adapter.getRemoteDevice(address)
        } catch (exception: IllegalArgumentException) {
            Log.e("BLEDevicesScannerImpl", "Invalid BLE address $address", exception)
            return@withContext null
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
            context.checkSelfPermission(android.Manifest.permission.BLUETOOTH_CONNECT) !=
            android.content.pm.PackageManager.PERMISSION_GRANTED
        ) {
            throw SecurityException("Missing BLUETOOTH_CONNECT permission")
        }

        readDeviceNameFromGatt(remoteDevice)
    }

    @SuppressLint("MissingPermission")
    private suspend fun readDeviceNameFromGatt(device: BluetoothDevice): String? =
        suspendCancellableCoroutine { continuation ->
            var completed = false
            var gatt: BluetoothGatt? = null

            fun finish(name: String?) {
                if (completed) {
                    return
                }
                completed = true
                gatt?.close()
                if (continuation.isActive) {
                    continuation.resume(name)
                }
            }

            val callback = object : BluetoothGattCallback() {
                override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                    when {
                        status != BluetoothGatt.GATT_SUCCESS -> finish(null)
                        newState == BluetoothProfile.STATE_CONNECTED -> {
                            val resolvedName = gatt.device.name
                            finish(resolvedName)
                            gatt.disconnect()
                        }
                        newState == BluetoothProfile.STATE_DISCONNECTED -> finish(null)
                    }
                }
            }

            gatt = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                device.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE)
            } else {
                device.connectGatt(context, false, callback)
            }

            continuation.invokeOnCancellation {
                gatt?.close()
            }
        }

    private fun updateResolvedDeviceName(address: BleAddress, resolvedName: String) {
        scanResultFlow.update { current ->
            current.copy(
                devices = current.devices.map { device ->
                    if (device.address == address) {
                        device.copy(name = resolvedName)
                    } else {
                        device
                    }
                },
                timestamp = System.currentTimeMillis()
            )
        }
    }
}
