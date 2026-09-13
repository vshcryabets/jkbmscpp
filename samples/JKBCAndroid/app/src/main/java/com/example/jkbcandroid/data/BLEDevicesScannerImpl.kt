package com.example.jkbcandroid.data

import android.annotation.SuppressLint
import android.bluetooth.BluetoothManager
import android.content.Context
import com.example.jkbcandroid.domain.BLEDevice
import com.example.jkbcandroid.domain.BLEDevicesScanner
import com.example.jkbcandroid.domain.SessionId
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.map
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicInteger
import javax.inject.Inject
import javax.inject.Singleton
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult

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
    private val devicesFlow = MutableStateFlow<Map<String, BLEDevice>>(emptyMap())

    private val sessions = ConcurrentHashMap<SessionId, Unit>()
    private val sessionIdCounter = AtomicInteger(0)

    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val device = BLEDevice(
                name = result.device.name ?: result.device.address,
                address = result.device.address,
                rssi = result.rssi
            )
            devicesFlow.value = devicesFlow.value + (device.address to device)
        }

        override fun onScanFailed(errorCode: Int) {
            // Scan failed — device list unchanged
        }
    }

    @SuppressLint("MissingPermission")
    override fun startScanSession(): SessionId {
        val id = sessionIdCounter.incrementAndGet()
        val wasEmpty = sessions.isEmpty()
        sessions[id] = Unit
        if (wasEmpty) {
            devicesFlow.value = emptyMap()
            bluetoothLeScanner?.startScan(scanCallback)
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

    override fun getScannedDevices(sessionId: SessionId): List<BLEDevice> {
        if (!sessions.containsKey(sessionId)) return emptyList()
        return devicesFlow.value.values.toList()
    }

    override fun observeScannedDevices(sessionId: SessionId): Flow<List<BLEDevice>> {
        return devicesFlow.map { map ->
            if (sessions.containsKey(sessionId)) map.values.toList() else emptyList()
        }
    }
}
