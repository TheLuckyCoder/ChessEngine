package net.theluckycoder.chess

import android.os.Bundle
import android.preference.PreferenceFragment
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity

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

            val preferences = Preferences(activity)

            findPreference(Preferences.KEY_RESET_TILE_COLORS).setOnPreferenceClickListener {
                preferences.whiteTileColor = getColor(activity, R.color.tile_white)
                preferences.blackTileColor = getColor(activity, R.color.tile_black)
                preferences.possibleTileColor = getColor(activity, R.color.tile_possible)
                preferences.selectedTileColor = getColor(activity, R.color.tile_selected)
                preferences.lastMovedTileColor = getColor(activity, R.color.tile_last_moved)
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