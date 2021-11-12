package net.theluckycoder.chess.wearos.ui

import androidx.compose.animation.ExperimentalAnimationApi
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.BoxScope
import androidx.compose.foundation.layout.Column
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.wear.compose.material.Button
import androidx.wear.compose.material.ExperimentalWearMaterialApi
import androidx.wear.compose.material.SwipeDismissTarget
import androidx.wear.compose.material.SwipeToDismissBox
import androidx.wear.compose.material.SwipeToDismissBoxDefaults
import androidx.wear.compose.material.Text
import androidx.wear.compose.material.rememberSwipeToDismissBoxState
import cafe.adriel.voyager.core.screen.Screen
import cafe.adriel.voyager.navigator.CurrentScreen
import cafe.adriel.voyager.navigator.Navigator

@Composable
fun TextIconButton(
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    enabled: Boolean = true,
    text: String,
    icon: @Composable BoxScope.() -> Unit
) {
    Column(
        modifier = modifier,
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Button(
            enabled = enabled,
            onClick = onClick,
            content = icon
        )

        Text(text = text)
    }
}

@ExperimentalAnimationApi
@OptIn(ExperimentalWearMaterialApi::class)
@Composable
fun SwipeDismissableNavigator(
    screen: Screen,
) = Navigator(screen = screen) { navigator ->
    val current = navigator.lastItem
    val previous = navigator.items.getOrNull(navigator.size - 2)

    val state = rememberSwipeToDismissBoxState()
    LaunchedEffect(state.currentValue) {
        // This effect operates when the swipe gesture is complete:
        // 1) Resets the screen offset (otherwise, the next destination is draw off-screen)
        // 2) Pops the navigation back stack to return to the previous level
        if (state.currentValue == SwipeDismissTarget.Dismissal) {
            state.snapTo(SwipeDismissTarget.Original)
            navigator.pop()
        }
    }

    SwipeToDismissBox(
        state = state,
        modifier = Modifier,
        hasBackground = navigator.canPop,
        backgroundKey = previous?.key ?: SwipeToDismissBoxDefaults.BackgroundKey,
        contentKey = current.key,
        content = { isBackground ->
            if (isBackground && previous != null)
                previous.Content()
            else {
                CurrentScreen()
            }
        }
    )
}

@Composable
fun isScreenRound(): Boolean {
    val ctx = LocalContext.current
    return remember { ctx.resources.configuration.isScreenRound }
}
