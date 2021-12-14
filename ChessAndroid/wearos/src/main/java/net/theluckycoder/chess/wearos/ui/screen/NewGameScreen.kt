package net.theluckycoder.chess.wearos.ui.screen

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.wear.compose.foundation.CurvedRow
import androidx.wear.compose.material.Button
import androidx.wear.compose.material.ButtonDefaults
import androidx.wear.compose.material.CurvedText
import androidx.wear.compose.material.ExperimentalWearMaterialApi
import androidx.wear.compose.material.Icon
import androidx.wear.compose.material.InlineSlider
import androidx.wear.compose.material.MaterialTheme
import androidx.wear.compose.material.PositionIndicator
import androidx.wear.compose.material.Scaffold
import androidx.wear.compose.material.ScalingLazyColumn
import androidx.wear.compose.material.Text
import androidx.wear.compose.material.Vignette
import androidx.wear.compose.material.VignettePosition
import androidx.wear.compose.material.rememberScalingLazyListState
import cafe.adriel.voyager.core.screen.Screen
import cafe.adriel.voyager.navigator.LocalNavigator
import cafe.adriel.voyager.navigator.currentOrThrow
import net.theluckycoder.chess.common.ui.ChooseSidesToggle
import net.theluckycoder.chess.common.viewmodel.HomeViewModel
import net.theluckycoder.chess.wearos.R
import kotlin.math.roundToInt
import kotlin.random.Random

class NewGameScreen : Screen {

    @OptIn(ExperimentalWearMaterialApi::class)
    @Composable
    override fun Content() {
        val viewModel: HomeViewModel = viewModel()

        val listState = rememberScalingLazyListState()

        Scaffold(
            timeText = {
                CurvedRow {
                    CurvedText(stringResource(R.string.new_game))
                }
            },
            positionIndicator = {
                PositionIndicator(listState)
            },
            vignette = {
                Vignette(vignettePosition = VignettePosition.TopAndBottom)
            }
        ) {
            val navigator = LocalNavigator.currentOrThrow

            val sidesToggleIndex = remember { mutableStateOf(0) }
            var difficultyLevel by remember { mutableStateOf(1f) }

            ScalingLazyColumn(
                modifier = Modifier.fillMaxSize(),
                state = listState,
                contentPadding = PaddingValues(vertical = 12.dp, horizontal = 6.dp)
            ) {
                item {
                    Spacer(modifier = Modifier.height(48.dp))
                }

                item {
                    Text(
                        text = stringResource(id = R.string.side),
                        color = MaterialTheme.colors.secondary,
                    )
                }

                item {
                    ChooseSidesToggle(
                        modifier = Modifier.padding(bottom = 8.dp),
                        sidesToggleIndex = sidesToggleIndex,
                        primaryColor = MaterialTheme.colors.primary
                    )
                }

                item {
                    Text(
                        text = stringResource(
                            R.string.difficulty_level,
                            difficultyLevel.roundToInt()
                        ),
                        color = MaterialTheme.colors.secondary,
                    )
                }

                item {
                    InlineSlider(
                        modifier = Modifier.padding(top = 4.dp),
                        value = difficultyLevel,
                        onValueChange = { difficultyLevel = it },
                        valueRange = 1f..5f,
                        steps = 3,
                    )
                }

                item {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(vertical = 8.dp),
                        horizontalArrangement = Arrangement.SpaceEvenly
                    ) {
                        Button(
                            colors = ButtonDefaults.secondaryButtonColors(),
                            onClick = {
                                navigator.pop()
                            }
                        ) {
                            Icon(
                                painter = painterResource(R.drawable.ic_close),
                                contentDescription = null
                            )
                        }

                        Button(onClick = {
                            val playerWhite = when (sidesToggleIndex.value) {
                                0 -> true
                                1 -> false
                                else -> Random.nextBoolean()
                            }

                            viewModel.updateDifficulty(difficultyLevel.roundToInt())

                            viewModel.resetBoard(playerWhite)
                            navigator.pop()
                        }) {
                            Icon(
                                painter = painterResource(R.drawable.ic_done),
                                contentDescription = null
                            )
                        }
                    }
                }

            }
        }
    }
}
