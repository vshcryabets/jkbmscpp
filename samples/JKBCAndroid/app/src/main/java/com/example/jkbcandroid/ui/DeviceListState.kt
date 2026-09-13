package com.example.jkbcandroid.ui

import androidx.compose.runtime.Immutable
import kotlinx.collections.immutable.ImmutableList
import kotlinx.collections.immutable.persistentListOf

@Immutable
data class DeviceListItem(
    val name: String,
    val address: String,
    val rssi: Int
)

@Immutable
data class DeviceListState(
    val items: ImmutableList<DeviceListItem> = persistentListOf(),
)