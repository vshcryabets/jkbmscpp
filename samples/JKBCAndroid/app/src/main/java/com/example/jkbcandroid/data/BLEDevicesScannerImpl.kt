package com.example.jkbcandroid.data

import android.annotation.SuppressLint
import android.bluetooth.BluetoothManager
import android.content.Context
import com.example.jkbcandroid.domain.BLEDevice
import com.example.jkbcandroid.domain.BLEDevicesScanner
import com.example.jkbcandroid.domain.ScanResult
import com.example.jkbcandroid.domain.SessionId
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicInteger
import javax.inject.Inject
import javax.inject.Singleton
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanSettings
import android.util.Log


@Singleton
class BLEDevicesScannerImpl @Inject constructor(
    @ApplicationContext private val context: Context
) : BLEDevicesScanner {

    private val bluetoothLeScanner by lazy {
        context.getSystemService(BluetoothManager::class.java)
            ?.adapter
            ?.bluetoothLeScanner
    }

    // Shared discovered devices map, keyed by address
    private val scanResultFlow = MutableStateFlow(
        ScanResult(
            devices = emptyList(),
            timestamp = System.currentTimeMillis(),
            error = null
        )
    )

    private val sessions = ConcurrentHashMap<SessionId, Unit>()
    private val sessionIdCounter = AtomicInteger(0)

    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: android.bluetooth.le.ScanResult) {
            Log.d("BLEDevicesScannerImpl", "Discovered device: ${result}")
            var advertisedName = result.scanRecord?.deviceName
            if (advertisedName.isNullOrEmpty()) {
                advertisedName = result.device.name
            }
            if (advertisedName.isNullOrEmpty()) {
                advertisedName = result.device.address
            }
            val device = BLEDevice(
                name = advertisedName,
                address = result.device.address,
                rssi = result.rssi
            )
            val devicesByAddress = scanResultFlow.value.devices.associateBy { it.address }.toMutableMap()
            devicesByAddress[device.address] = device
            scanResultFlow.value = ScanResult(
                devices = devicesByAddress.values.toList(),
                timestamp = System.currentTimeMillis(),
                error = null
            )
        }

        override fun onScanFailed(errorCode: Int) {
            scanResultFlow.value = ScanResult(
                devices = scanResultFlow.value.devices,
                timestamp = System.currentTimeMillis(),
                error = IllegalStateException("BLE scan failed with error code: $errorCode")
            )
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
                val settings = ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY) // Forces active scanning to fetch SCAN_RSP
                    .build()
                bluetoothLeScanner?.startScan(null, settings, scanCallback)
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
}
