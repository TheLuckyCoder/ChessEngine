package net.theluckycoder.chess.ui

import android.content.ClipData
import android.content.ClipboardManager
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.activity.viewModels
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.material.Surface
import androidx.compose.ui.platform.ComposeView
import androidx.core.content.getSystemService
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.databinding.DialogImportFenBinding
import kotlin.random.Random

class ComposeActivity : AppCompatActivity() {

    private val chessViewModel by viewModels<ChessViewModel>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val view = ComposeView(this).apply {
            setContent {
                ChessMaterialTheme {
                    Surface {
                        MainScreen()
                    }
                }
            }
        }

        setContentView(view)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_main, menu)
        return super.onCreateOptionsMenu(menu)
    }

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
