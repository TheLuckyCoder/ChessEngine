package net.theluckycoder.chess

import android.os.Bundle
import android.util.Log
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.Preference
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.SeekBarPreference
import net.theluckycoder.chess.utils.AppPreferences
import net.theluckycoder.chess.utils.getColorCompat
import kotlin.concurrent.thread
import kotlin.math.min

class SettingsActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        supportFragmentManager
            .beginTransaction()
            .replace(android.R.id.content, SettingsFragment())
            .commit()
    }

    class SettingsFragment : PreferenceFragmentCompat() {

        override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
            addPreferencesFromResource(R.xml.preferences)

            findPreference<Preference>(AppPreferences.KEY_RESET_APPEARANCE)?.setOnPreferenceClickListener {
                val context = requireContext()

                AppPreferences(context).apply {
                    whiteTileColor = context.getColorCompat(R.color.tile_white)
                    blackTileColor = context.getColorCompat(R.color.tile_black)
                    possibleMoveColor = context.getColorCompat(R.color.tile_possible)
                    lastMovedTileColor = context.getColorCompat(R.color.tile_last_moved)
                    inCheckColor = context.getColorCompat(R.color.king_in_check)
                    showCoordinates = true
                }
                true
            }

            val threadCountPref = findPreference<SeekBarPreference>(AppPreferences.KEY_THREAD_COUNT)
            if (threadCountPref != null) {
                val defaultValue = min(Runtime.getRuntime().availableProcessors() - 1, 1)

                threadCountPref.setDefaultValue(defaultValue)
                threadCountPref.max = Runtime.getRuntime().availableProcessors()
                if (threadCountPref.value == 0)
                    threadCountPref.value = defaultValue
            }

            findPreference<Preference>(AppPreferences.KEY_PERFT_TEST)?.setOnPreferenceClickListener {
                thread {
                    Native.perftTest()
                }
                true
            }

            findPreference<Preference>(AppPreferences.KEY_EVALUATION_TEST)?.setOnPreferenceClickListener {
                thread {
                    val result = Native.evaluationTest() ?: "Completed Successfully"
                    Log.v("Evaluation Test", result)
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