package net.theluckycoder.chess.ui

import android.app.Activity
import android.content.Context
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.dimensionResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.sp
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.ui.preferences.*
import net.theluckycoder.chess.utils.SettingsDataStore
import net.theluckycoder.chess.utils.browseUrl
import net.theluckycoder.chess.utils.settingsDataStore

class PreferencesActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val view = ComposeView(this).apply {
            setContent {
                ChessMaterialTheme {
                    Preferences()
                }
            }
        }

        setContentView(view)
    }
}

@OptIn(ExperimentalMaterialApi::class)
@Composable
private fun Preferences() = Scaffold(
    topBar = { AppBar() }
) { padding ->
    val context = LocalContext.current

    Box(modifier = Modifier.padding(padding)) {
        PreferenceScreen(
            dataStore = context.settingsDataStore,
            items = getPreferenceItems(context),
        )
    }
}

@Composable
private fun getPreferenceItems(
    context: Context
) = listOf(
    PreferenceGroupItem(
        title = stringResource(id = R.string.pref_category_difficulty),
        items = listOf(
            SeekbarIntPreferenceItem(
                title = stringResource(id = R.string.pref_search_depth),
                summary = stringResource(id = R.string.pref_search_depth_desc),
                prefKey = SettingsDataStore.SEARCH_DEPTH,
                icon = painterResource(id = R.drawable.ic_pref_search),
                steps = 10,
                defaultValue = SettingsDataStore.DEFAULT_SEARCH_DEPTH,
                valueRange = 2..13,
            ),
            SwitchPreferenceItem(
                title = stringResource(id = R.string.pref_quiet_search),
                summary = stringResource(id = R.string.pref_quiet_search_desc),
                prefKey = SettingsDataStore.QUIET_SEARCH,
                icon = painterResource(id = R.drawable.ic_pref_quiet_search),
                defaultValue = SettingsDataStore.DEFAULT_QUIET_SEARCH,
            ),
        ),
    ),

    PreferenceGroupItem(
        title = stringResource(id = R.string.pref_category_other),
        items = listOf(
            SeekbarIntPreferenceItem(
                title = stringResource(id = R.string.pref_thread_count),
                summary = stringResource(id = R.string.pref_thread_count_desc),
                prefKey = SettingsDataStore.THREADS,
                icon = painterResource(id = R.drawable.ic_pref_quiet_search),
                valueRange = 1..Runtime.getRuntime().availableProcessors(),
                defaultValue = SettingsDataStore.DEFAULT_THREADS
            ),
            SeekbarIntPreferenceItem(
                title = stringResource(id = R.string.pref_cache_size),
                summary = stringResource(id = R.string.pref_cache_size_desc),
                prefKey = SettingsDataStore.HASH_SIZE,
                icon = painterResource(id = R.drawable.ic_pref_cache),
                valueRange = 32..512,
                defaultValue = SettingsDataStore.DEFAULT_HASH_SIZE
            ),
        ),
    ),

    PreferenceGroupItem(
        title = stringResource(id = R.string.pref_category_about),
        items = listOf(
            EmptyPreferenceItem(
                title = stringResource(id = R.string.about_author),
                summary = "TheLuckyCoder - Filea Răzvan Gheorghe",
                icon = painterResource(id = R.drawable.ic_pref_author),
                onClick = { context.browseUrl("https://theluckycoder.net") },
            ),
            EmptyPreferenceItem(
                title = stringResource(id = R.string.about_source_code),
                summary = "Licensed under the GNU General Public License",
                icon = painterResource(id = R.drawable.ic_pref_source_code),
                onClick = { context.browseUrl("https://github.com/TheLuckyCoder/ChessEngine") }
            ),
        ),
    ),

    PreferenceGroupItem(
        title = stringResource(id = R.string.pref_category_debug),
        items = listOf(
            SwitchPreferenceItem(
                title = stringResource(id = R.string.pref_debug_basic),
                summary = stringResource(id = R.string.pref_debug_basic_desc),
                prefKey = SettingsDataStore.SHOW_DEBUG_BASIC,
                icon = painterResource(id = R.drawable.ic_pref_debug_info),
                defaultValue = false,
            ),
            SwitchPreferenceItem(
                title = stringResource(id = R.string.pref_debug_advanced),
                summary = stringResource(id = R.string.pref_debug_advanced_desc),
                prefKey = SettingsDataStore.SHOW_DEBUG_ADVANCED,
                icon = painterResource(id = R.drawable.ic_pref_stats),
                defaultValue = false,
            ),
            EmptyPreferenceItem(
                title = "Run Perft Test",
                summary = "This is only meant for debugging",
                onClick = { Native.perftTest() }
            ),
            EmptyPreferenceItem(
                title = "Run Evaluation Test",
                summary = "This is only meant for debugging",
                onClick = { Native.evaluationTest() }
            ),
        ),
    ),
)

@Preview
@Composable
private fun AppBar() {
    TopAppBar(
        modifier = Modifier.height(dimensionResource(id = R.dimen.toolbar_height)),
        backgroundColor = MaterialTheme.colors.primary,
        title = {
            Text(
                text = stringResource(id = R.string.title_settings),
                fontSize = 20.sp,
                fontWeight = FontWeight.SemiBold,
            )
        },
        navigationIcon = {
            val activity = LocalContext.current as Activity
            IconButton(onClick = { activity.finish() }) {
                Icon(
                    painter = painterResource(id = R.drawable.ic_arrow_back),
                    contentDescription = null
                )
            }
        }
    )
}
