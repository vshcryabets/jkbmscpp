package com.example.jkbcandroid.ui

import com.example.jkbcandroid.domain.BLEDevice
import kotlinx.collections.immutable.ImmutableList
import kotlinx.collections.immutable.toImmutableList

fun List<BLEDevice>.toUi(): ImmutableList<DeviceListItem> {
    return this.map { device ->
        DeviceListItem(
            name = device.name,
            address = device.address,
            rssi = device.rssi
        )
    }.toImmutableList()
}