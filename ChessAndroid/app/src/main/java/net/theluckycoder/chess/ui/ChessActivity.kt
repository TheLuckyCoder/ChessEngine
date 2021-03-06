package net.theluckycoder.chess.ui

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.viewModels
import androidx.compose.ui.platform.ComposeView
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.launch
import net.theluckycoder.chess.ChessViewModel

class ChessActivity : ComponentActivity() {

    private val chessViewModel by viewModels<ChessViewModel>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val view = ComposeView(this).apply {
            setContent {
                ChessMaterialTheme {
                    MainScreen()
                }
            }
        }

        setContentView(view)
    }

    override fun onStart() {
        super.onStart()

        lifecycleScope.launch {
            ensureActive()
            chessViewModel.updateEngineSettings()
        }
    }
}
