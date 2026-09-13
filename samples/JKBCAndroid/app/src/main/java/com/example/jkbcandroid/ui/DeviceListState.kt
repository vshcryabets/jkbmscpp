package com.example.jkbcandroid.ui

import androidx.compose.runtime.Immutable
import kotlinx.collections.immutable.ImmutableList
import kotlinx.collections.immutable.persistentListOf

@Immutable
data class DeviceListItem(
    val name: String,
    val hasNoName: Boolean = false,
    val address: String,
    val rssi: Int
)

@Immutable
sealed interface UiDialog {
    object None: UiDialog
}

@Immutable
data class DeviceListState(
    val items: ImmutableList<DeviceListItem> = persistentListOf(),
    val requestPermissions: Boolean = false,
    val dialog: UiDialog = UiDialog.None,
)