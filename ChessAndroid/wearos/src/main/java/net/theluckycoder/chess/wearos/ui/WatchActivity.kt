package net.theluckycoder.chess.wearos.ui

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.animation.ExperimentalAnimationApi

class WatchActivity : ComponentActivity() {

    @OptIn(ExperimentalAnimationApi::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            ChessMaterialTheme {
                SwipeDismissableNavigator(WatchScreen)
//                Navigator(WatchScreen) {
//                    SlideTransition(it)
//                }
            }
        }
    }
}
