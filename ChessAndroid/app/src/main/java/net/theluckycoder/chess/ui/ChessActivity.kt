package net.theluckycoder.chess.ui

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.viewModels
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material.Surface
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ComposeView
import net.theluckycoder.chess.ChessViewModel

class ChessActivity : ComponentActivity() {

    private val chessViewModel by viewModels<ChessViewModel>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val view = ComposeView(this).apply {
            setContent {
                ChessMaterialTheme {
                    Surface(modifier = Modifier.fillMaxSize()) {
                        MainScreen()
                    }
                }
            }
        }

        setContentView(view)
    }
/*
    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_load_fen -> {
                val dialogBinding = DialogImportFenBinding.inflate(layoutInflater)

                val dialog = AlertDialog.Builder(this)
                    .setTitle(R.string.action_load_fen)
                    .setView(dialogBinding.root)
                    .create()

                dialogBinding.btnCancel.setOnClickListener { dialog.dismiss() }

                dialogBinding.btnLoad.setOnClickListener {
                    val fenPosition = dialogBinding.etFen.text?.toString()

                    if (fenPosition.isNullOrBlank()) {
                        dialogBinding.etFen.error = getString(R.string.error_empty_text_field)
                        return@setOnClickListener
                    }

                    val playerWhite = when (dialogBinding.tgSide.checkedButtonId) {
                        dialogBinding.btnSideWhite.id -> true
                        dialogBinding.btnSideBlack.id -> false
                        else -> Random.nextBoolean()
                    }

                    if (Native.loadFen(playerWhite, fenPosition)) {
                        chessViewModel.initBoard(playerWhite)
                        Native.loadFen(playerWhite, fenPosition)

                        Toast.makeText(this, R.string.fen_position_loaded, Toast.LENGTH_SHORT)
                            .show()
                        dialog.dismiss()
                    } else {
                        dialogBinding.etFen.error = getString(R.string.error_fen_position_invalid)
                        Toast.makeText(this, R.string.fen_position_error, Toast.LENGTH_LONG)
                            .show()
                    }
                }

                dialog.show()
            }
            R.id.action_export_fen -> {
                val fen = Native.getFen()

                AlertDialog.Builder(this)
                    .setTitle(R.string.fen_position_exported)
                    .setMessage(fen)
                    .setNeutralButton(android.R.string.copy) { _, _ ->
                        val clipboard = getSystemService<ClipboardManager>()
                        val clip = ClipData.newPlainText("FEN Position", fen)
                        clipboard!!.setPrimaryClip(clip)
                        Toast.makeText(this, R.string.fen_position_copied, Toast.LENGTH_SHORT)
                            .show()
                    }
                    .setNegativeButton(android.R.string.cancel, null)
                    .show()
            }
            R.id.action_force_move -> {
                Native.forceMove()
//                showLoadingBar()
            }
        }
        return super.onOptionsItemSelected(item)
    }
*/
    override fun onStart() {
        super.onStart()

        with(chessViewModel) {
            val showDebugInfo = preferences.basicDebugInfo
            if (basicStatsEnabled != showDebugInfo) {
                basicStatsEnabled = showDebugInfo
                invalidateOptionsMenu()
            }
            advancedStatsEnabled = preferences.advancedDebugInfo
            updateSettings(preferences.engineSettings)
        }
    }
}
