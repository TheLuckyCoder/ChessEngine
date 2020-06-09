package net.theluckycoder.chess

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.util.Log
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.*
import com.marcoscg.licenser.Library
import com.marcoscg.licenser.License
import com.marcoscg.licenser.LicenserDialog
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

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return if (item.itemId == android.R.id.home) {
            onBackPressed()
            true
        } else super.onOptionsItemSelected(item)
    }

    class SettingsFragment : PreferenceFragmentCompat() {

        override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
            addPreferencesFromResource(R.xml.preferences)

            findPreference<Preference>(AppPreferences.KEY_RESET_APPEARANCE)!!.setOnPreferenceClickListener {
                val context = requireContext()

                with(AppPreferences) {
                    whiteTileColor = context.getColorCompat(R.color.tile_white)
                    blackTileColor = context.getColorCompat(R.color.tile_black)
                    possibleMoveColor = context.getColorCompat(R.color.tile_possible)
                    lastMovedTileColor = context.getColorCompat(R.color.tile_last_moved)
                    inCheckColor = context.getColorCompat(R.color.king_in_check)
                    showCoordinates = true
                }

                true
            }

            findPreference<SeekBarPreference>(AppPreferences.KEY_THREAD_COUNT)!!.let { threadCountPref ->
                val defaultValue = min(Runtime.getRuntime().availableProcessors() - 1, 1)

                threadCountPref.setDefaultValue(defaultValue)
                threadCountPref.max = Runtime.getRuntime().availableProcessors()

                if (threadCountPref.value == 0)
                    threadCountPref.value = defaultValue
            }

            // About Category
            val authorPref = findPreference<Preference>(AppPreferences.KEY_ABOUT_AUTHOR)!!
            authorPref.setOnPreferenceClickListener {
                launchUrl(AUTHOR_URL)
                true
            }

            findPreference<Preference>(AppPreferences.KEY_ABOUT_SOURCE_CODE)!!.let { sourceCodePref ->
                sourceCodePref.summary = SOURCE_CODE_URL
                sourceCodePref.setOnPreferenceClickListener {
                    launchUrl(SOURCE_CODE_URL)
                    true
                }
            }

            val licensesPref = findPreference<Preference>(AppPreferences.KEY_ABOUT_LICENSES)!!
            licensesPref.setOnPreferenceClickListener {
                showLicenses()
                true
            }

            // Debug Category
            val perftTestPref = findPreference<Preference>(AppPreferences.KEY_PERFT_TEST)!!
            perftTestPref.setOnPreferenceClickListener {
                thread {
                    Native.perftTest()
                }
                true
            }

            val evaluationTestPref =
                findPreference<Preference>(AppPreferences.KEY_EVALUATION_TEST)!!
            evaluationTestPref.setOnPreferenceClickListener {
                thread {
                    val result = Native.evaluationTest() ?: "Completed Successfully"
                    Log.v("Evaluation Test", result)
                }
                true
            }

            val debugInfoPref =
                findPreference<SwitchPreferenceCompat>(AppPreferences.KEY_DEBUG_INFO_BASIC)!!

            perftTestPref.isVisible = debugInfoPref.isChecked
            evaluationTestPref.isVisible = debugInfoPref.isChecked

            debugInfoPref.setOnPreferenceChangeListener { _, newValue ->
                val value = newValue == true

                perftTestPref.isVisible = value
                evaluationTestPref.isVisible = value
                true
            }
        }

        private fun launchUrl(url: String) {
            val browserIntent = Intent(Intent.ACTION_VIEW, Uri.parse(url))
            startActivity(browserIntent)
        }

        private fun showLicenses() {
            LicenserDialog(context)
                .setLibrary(
                    Library(
                        "AndroidX",
                        "https://developer.android.com/jetpack/androidx/",
                        License.APACHE2
                    )
                )
                .setLibrary(
                    Library(
                        "ColorPicker",
                        "https://github.com/jaredrummler/ColorPicker",
                        License.APACHE2
                    )
                )
                .setLibrary(
                    Library(
                        "Licenser",
                        "https://github.com/marcoscgdev/Licenser",
                        License.MIT
                    )
                )
                .show()
        }

        companion object {
            const val AUTHOR_URL = "http://theluckycoder.net/"
            const val SOURCE_CODE_URL = "https://github.com/TheLuckyCoder/ChessEngine"
        }
    }
}