package com.example.jkbcandroid.ui

import android.Manifest
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.toRoute

@Composable
fun AppNavigation() {
    val navController = rememberNavController()

    NavHost(
        navController = navController,
        startDestination = ListRoute
    ) {
        // BLE devices list
        composable<ListRoute> {
            val viewModel: DeviceListViewModel = hiltViewModel()
            val state by viewModel.state.collectAsStateWithLifecycle(DeviceListState())


            val permissionLauncher = rememberLauncherForActivityResult(
                contract = ActivityResultContracts.RequestMultiplePermissions()
            ) { permissions ->
                val hasScanPermission = permissions[Manifest.permission.BLUETOOTH_SCAN] == true
                val hasConnectPermission = permissions[Manifest.permission.BLUETOOTH_CONNECT] == true
                viewModel.onPermissionResult(hasScanPermission && hasConnectPermission)
            }

            DeviceListScreen(
                state = state,
                onItemClick = { item ->
                    viewModel.onUiEvent(UiEvent.OnItemClick(item.address))
                },
                onRequestPermission = {
                    permissionLauncher.launch(
                        arrayOf(
                            Manifest.permission.BLUETOOTH_SCAN,
                            Manifest.permission.BLUETOOTH_CONNECT,
                        )
                    )
                },
            )
        }

        // Details screen
        composable<DetailRoute> { backStackEntry ->
            // Automatically get parameters with type safety
            val route: DetailRoute = backStackEntry.toRoute()

            DetailsScreen(
            )
        }
    }
}