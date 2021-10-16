package net.theluckycoder.chess.wearos.ui

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.compose.ui.platform.ComposeView

class WatchActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val view = ComposeView(this).apply {
            setContent {
                ChessMaterialTheme {
                    WatchScreen()
                }
            }
        }

        setContentView(view)
    }
}
