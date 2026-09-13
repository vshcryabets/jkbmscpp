package com.example.jkbcandroid.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.example.jkbcandroid.ui.theme.JKBCAndroidTheme
import kotlinx.collections.immutable.persistentListOf

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DeviceListScreen(
    modifier: Modifier = Modifier,
    state: DeviceListState = DeviceListState(),
    onItemClick: (DeviceListItem) -> Unit,
    onRequestPermission: () -> Unit,
) {
    Scaffold(
        modifier = modifier.fillMaxSize(),
        topBar = {
            TopAppBar(title = { Text("Devices") })
        }
    ) { innerPadding ->
        if (state.requestPermissions) {
            PermissionRequiredContent(
                modifier = Modifier.padding(innerPadding),
                onRequestPermission = onRequestPermission,
            )
        } else {
            DeviceListContent(
                state = state,
                modifier = Modifier.padding(innerPadding),
                onItemClick = onItemClick,
            )
        }
    }
}

@Composable
private fun DeviceListContent(
    state: DeviceListState,
    modifier: Modifier = Modifier,
    onItemClick: (DeviceListItem) -> Unit,
) {
    LazyColumn(modifier = modifier) {
        itemsIndexed(state.items) { index, item ->
            Column() {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp, 4.dp)
                ) {
                    if (item.hasNoName) {
                        Text(
                            text = item.name,
                            color = androidx.compose.ui.graphics.Color.Gray,
                        )
                    } else {
                        Text(
                            text = item.name,
                            color = androidx.compose.ui.graphics.Color.Black,
                            fontWeight = FontWeight.Bold,
                        )
                    }
                    Text(text = item.address,
                        modifier = Modifier.padding(start = 8.dp),
                        color = androidx.compose.ui.graphics.Color.Gray
                        )
                }
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp, 4.dp)
                ) {
                    Text(text = "RSSI: ${item.rssi}")
                }
                if (index < state.items.lastIndex) {
                    HorizontalDivider(
                        modifier = Modifier.padding(horizontal = 8.dp), // Optional: indents the line
                        thickness = 1.dp,
                        color = androidx.compose.ui.graphics.Color.LightGray
                    )
                }
            }
        }
    }
}

@Composable
private fun PermissionRequiredContent(
    modifier: Modifier = Modifier,
    onRequestPermission: () -> Unit,
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.Center,
    ) {
        Text("Bluetooth permission is required to scan for nearby devices.")
        Spacer(modifier = Modifier.height(16.dp))
        Button(
            onClick = onRequestPermission,
            modifier = Modifier.fillMaxWidth(),
        ) {
            Text("Grant Permission")
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
            ),
            onItemClick = {},
        )
    }
}