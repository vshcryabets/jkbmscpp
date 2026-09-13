package com.example.jkbcandroid.ui

import com.example.jkbcandroid.domain.BLEDevice
import kotlinx.collections.immutable.ImmutableList
import kotlinx.collections.immutable.toImmutableList

fun List<BLEDevice>.toUi(): ImmutableList<DeviceListItem> {
    return this
        .sortedWith(
            compareBy<BLEDevice> { it.name.isBlank() }
                .thenBy { it.name }
        )
        .map { device ->
            DeviceListItem(
                name = if (device.name.isBlank()) "N/A" else device.name,
                hasNoName = device.name.isBlank(),
                address = device.address,
                rssi = device.rssi
            )
        }
        .toImmutableList()
}