package com.example.jkbcandroid.domain.usecases

import com.example.jkbcandroid.domain.BLEDevice
import kotlinx.coroutines.flow.Flow

interface ObserveBleDevicesUseCase {
    fun execute(): Flow<List<BLEDevice>>
}