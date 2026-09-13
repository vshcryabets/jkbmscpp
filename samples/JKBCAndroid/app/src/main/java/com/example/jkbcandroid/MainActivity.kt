package com.example.jkbcandroid

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.ui.Modifier
import com.example.jkbcandroid.ui.AppNavigation
import com.example.jkbcandroid.ui.DeviceListScreen
import dagger.hilt.android.AndroidEntryPoint
import com.example.jkbcandroid.ui.theme.JKBCAndroidTheme

@AndroidEntryPoint
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            JKBCAndroidTheme {
                AppNavigation()
            }
        }
    }
}
