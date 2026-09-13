package com.example.jkbcandroid.ui

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.example.jkbcandroid.ui.theme.JKBCAndroidTheme
import kotlinx.collections.immutable.persistentListOf

@Composable
fun DeviceListScreen(modifier: Modifier = Modifier) {
    DeviceListContent(state = DeviceListState(), modifier = modifier)
}

@Composable
private fun DeviceListContent(state: DeviceListState, modifier: Modifier = Modifier) {
    LazyColumn(modifier = modifier) {
        items(state.items) { item ->
            Column {
                Text(text = item.name)
                Text(text = item.address)
                Text(text = "RSSI: ${item.rssi}")
            }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun DeviceListScreenPreview() {
    JKBCAndroidTheme {
        DeviceListContent(
            state = DeviceListState(
                items = persistentListOf(
                    DeviceListItem(name = "Demo Device", address = "00:11:22:33:44:55", rssi = -52)
                )
            )
        )
    }
}