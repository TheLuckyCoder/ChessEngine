package net.theluckycoder.chess.wearos.ui.screen

import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.wear.compose.foundation.CurvedRow
import androidx.wear.compose.material.CurvedText
import androidx.wear.compose.material.PositionIndicator
import androidx.wear.compose.material.Scaffold
import androidx.wear.compose.material.ScalingLazyColumn
import androidx.wear.compose.material.Vignette
import androidx.wear.compose.material.VignettePosition
import androidx.wear.compose.material.rememberScalingLazyListState
import cafe.adriel.voyager.core.screen.Screen
import net.theluckycoder.chess.wearos.R

class SettingsScreen : Screen {

    @Composable
    override fun Content() {
        val listState = rememberScalingLazyListState()

        Scaffold(
            timeText = {
                CurvedRow {
                    CurvedText(stringResource(R.string.title_settings))
                }
            },
            positionIndicator = { PositionIndicator(listState) },
            vignette = { Vignette(VignettePosition.TopAndBottom) },
        ) {
            ScalingLazyColumn(
                modifier = Modifier.fillMaxSize(),
                state = listState,
                contentPadding = PaddingValues(vertical = 12.dp, horizontal = 6.dp)
            ) {
                item {
                    Spacer(modifier = Modifier.height(48.dp))
                }
            }
        }
    }
}
