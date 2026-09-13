package com.example.jkbcandroid.domain.usecases

import com.example.jkbcandroid.domain.BLEDevice
import com.example.jkbcandroid.domain.ScanResult
import kotlinx.coroutines.flow.Flow

interface ObserveBleDevicesUseCase {
    fun execute(): Flow<ScanResult>
}