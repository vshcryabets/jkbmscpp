package com.example.jkbcandroid.domain

import kotlinx.coroutines.flow.Flow

typealias SessionId = Int
typealias BleAddress = String

data class BLEDevice(
    val name: String,
    val address: BleAddress,
    val rssi: Int
)

interface BLEDevicesScanner {
    fun startScanSession(): SessionId
    fun stopScanSession(sessionId: SessionId): Boolean
    fun getScannedDevices(sessionId: SessionId): List<BLEDevice>
    fun observeScannedDevices(sessionId: SessionId): Flow<List<BLEDevice>>
}