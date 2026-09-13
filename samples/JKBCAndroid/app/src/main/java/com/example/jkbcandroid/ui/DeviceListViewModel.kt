package com.example.jkbcandroid.ui

import android.util.Log
import androidx.lifecycle.ViewModel
import com.example.jkbcandroid.domain.usecases.ObserveBleDevicesUseCase
import dagger.hilt.android.lifecycle.HiltViewModel
import jakarta.inject.Inject
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.combine
import kotlinx.coroutines.flow.flatMapLatest
import kotlinx.coroutines.flow.update

sealed interface UiEvent {
    data class OnItemClick(val deviceId: String) : UiEvent
}
@HiltViewModel
class DeviceListViewModel @Inject constructor(
    private val observeBleDevicesUseCase: ObserveBleDevicesUseCase,
): ViewModel() {
    private val _state = MutableStateFlow(DeviceListState())
    private val scanRestartTrigger = MutableStateFlow(Unit) // Trigger to restart scanning
    val state = combine(
        _state,
        scanRestartTrigger.flatMapLatest {
            // Викликається при старті та при кожній зміні scanRestartTrigger
            observeBleDevicesUseCase.execute()
        }
    ) { innerState, scanResult ->
        if (scanResult.error != null) {
            handleScanError(scanResult.error)
        }
//        Log.d("DeviceListViewModel", "Scan result: ${scanResult.devices}")
        DeviceListState(
            items = scanResult.devices.toUi(),
            requestPermissions = innerState.requestPermissions,
            dialog = innerState.dialog,
        )
    }

    private fun handleScanError(error: Exception) {
        Log.e("DeviceListViewModel", "Error scanning BLE devices: ${error.message}", error)
        if (error is SecurityException) {
            // Handle permission error, e.g., notify the user to grant permissions
            _state.update {
                it.copy(requestPermissions = true)
            }
        } else {
            // Handle other types of errors
        }
    }

    fun onUiEvent(event: UiEvent) {
        when (event) {
            is UiEvent.OnItemClick -> {
                // Handle item click event
            }
        }
    }

    fun onPermissionResult(granted: Boolean) {
        Log.d("DeviceListViewModel", "Permission result: $granted")
        if (granted) {
            scanRestartTrigger.tryEmit(Unit)
        }
        _state.update {
            it.copy(requestPermissions = !granted)
        }
    }
}