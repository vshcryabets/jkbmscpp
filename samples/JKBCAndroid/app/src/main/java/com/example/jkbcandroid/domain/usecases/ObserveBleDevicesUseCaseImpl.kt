package com.example.jkbcandroid.domain.usecases

import android.os.Build
import android.util.Log
import com.example.jkbcandroid.domain.BLEDevicesScanner
import com.example.jkbcandroid.domain.ScanResult
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import javax.inject.Inject

class ObserveBleDevicesUseCaseImpl @Inject constructor(
    private val scanner: BLEDevicesScanner,
    @ApplicationContext private val context: android.content.Context
) : ObserveBleDevicesUseCase {

    override fun execute(): Flow<ScanResult> = flow {
        val hasScanPermission = Build.VERSION.SDK_INT < Build.VERSION_CODES.S ||
                context.checkSelfPermission(android.Manifest.permission.BLUETOOTH_SCAN) == android.content.pm.PackageManager.PERMISSION_GRANTED
        val hasConnectPermission = Build.VERSION.SDK_INT < Build.VERSION_CODES.S ||
                context.checkSelfPermission(android.Manifest.permission.BLUETOOTH_CONNECT) == android.content.pm.PackageManager.PERMISSION_GRANTED

        if (!hasScanPermission || !hasConnectPermission) {
            Log.e("ObserveBleDevicesUseCaseImpl", "Missing required Bluetooth permissions")
            emit(
                ScanResult(
                    devices = emptyList(),
                    timestamp = System.currentTimeMillis(),
                    error = SecurityException("Missing required Bluetooth permissions")
                )
            )
            return@flow
        }

        val activeSessionId = scanner.startScanSession()
        try {
            scanner.observeScannedDevices(activeSessionId).collect { emit(it) }
        } finally {
            scanner.stopScanSession(activeSessionId)
        }
    }
}
