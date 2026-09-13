package com.example.jkbcandroid.ui

import androidx.lifecycle.ViewModel
import com.example.jkbcandroid.domain.usecases.ObserveBleDevicesUseCase
import jakarta.inject.Inject
import kotlinx.coroutines.flow.map

class DeviceListViewModel @Inject constructor(
    private val observeBleDevicesUseCase: ObserveBleDevicesUseCase,
): ViewModel() {
    val state = observeBleDevicesUseCase
        .execute()
        .map { devicesList ->
            DeviceListState(
                items = devicesList.toUi()
            )
        }
}