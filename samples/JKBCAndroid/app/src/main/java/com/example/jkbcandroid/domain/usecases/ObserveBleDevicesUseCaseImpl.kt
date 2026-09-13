package com.example.jkbcandroid.domain.usecases

import com.example.jkbcandroid.domain.BLEDevicesScanner
import com.example.jkbcandroid.domain.ScanResult
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import javax.inject.Inject

class ObserveBleDevicesUseCaseImpl @Inject constructor(
    private val scanner: BLEDevicesScanner
) : ObserveBleDevicesUseCase {

    override fun execute(): Flow<ScanResult> = flow {
        val activeSessionId = scanner.startScanSession()
        try {
            scanner.observeScannedDevices(activeSessionId).collect { emit(it) }
        } finally {
            scanner.stopScanSession(activeSessionId)
        }
    }
}
