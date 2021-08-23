package net.theluckycoder.chess.wearos.ui

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.compose.ui.platform.ComposeView
import net.theluckycoder.chess.common.ui.ChessMaterialTheme

class WatchActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val view = ComposeView(this).apply {
            setContent {
                ChessMaterialTheme {
                    HomeChessBoard()
                }
            }
        }

        setContentView(view)

    }
}