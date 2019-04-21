package net.theluckycoder.chess

import android.os.Bundle
import android.preference.PreferenceFragment
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import net.theluckycoder.chess.utils.getColor

class SettingsActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        fragmentManager.beginTransaction().replace(android.R.id.content, SettingsFragment()).commit()
    }

    class SettingsFragment : PreferenceFragment() {

        override fun onCreate(savedInstanceState: Bundle?) {
            super.onCreate(savedInstanceState)
            addPreferencesFromResource(R.xml.preferences)

            findPreference(Preferences.KEY_RESET_COLORS).setOnPreferenceClickListener {
                val activity = activity ?: return@setOnPreferenceClickListener false

                Preferences(activity).apply {
                    whiteTileColor = getColor(activity, R.color.tile_white)
                    blackTileColor = getColor(activity, R.color.tile_black)
                    possibleTileColor = getColor(activity, R.color.tile_possible)
                    selectedTileColor = getColor(activity, R.color.tile_selected)
                    lastMovedTileColor = getColor(activity, R.color.tile_last_moved)
                    kingInChessColor = getColor(activity, R.color.king_in_chess)
                }
                true
            }
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return if (item.itemId == android.R.id.home) {
            onBackPressed()
            true
        } else super.onOptionsItemSelected(item)
    }
}