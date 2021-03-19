package net.theluckycoder.chess.utils

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.intPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map
import net.theluckycoder.chess.model.EngineSettings
import java.util.concurrent.TimeUnit
import kotlin.time.ExperimentalTime
import kotlin.time.seconds

val Context.settingsDataStore: DataStore<Preferences> by preferencesDataStore(
    name = "settings",
)

@OptIn(ExperimentalTime::class)
class SettingsDataStore(private val context: Context) {

    private fun dataStore() = context.settingsDataStore

    fun firstStart(): Flow<Boolean> =
        dataStore().data.map { it[FIRST_START] ?: true }

    suspend fun setFirstStart(value: Boolean) = dataStore().edit { preferences ->
        preferences[FIRST_START] = value
    }

    fun showCoordinates(): Flow<Boolean> =
        dataStore().data.map { it[SHOW_COORDINATES] ?: DEFAULT_SHOW_COORDINATES }

    fun showMoveHistory(): Flow<Boolean> =
        dataStore().data.map { it[SHOW_MOVES_HISTORY] ?: DEFAULT_MOVES_HISTORY }

    suspend fun setDifficultyLevel(value: Int) = dataStore().edit { preferences ->
        preferences[SEARCH_DEPTH] = if (value == 0 || value == 1) value + 2 else value + 3
        preferences[QUIET_SEARCH] = value != 0
    }

    fun getEngineSettings(): Flow<EngineSettings> =
        dataStore().data.map {
            EngineSettings(
                searchDepth = it[SEARCH_DEPTH] ?: DEFAULT_SEARCH_DEPTH,
                quietSearch = it[QUIET_SEARCH] ?: DEFAULT_QUIET_SEARCH,
                searchTime = (it[SEARCH_TIME] ?: DEFAULT_SEARCH_TIME).seconds,
                threadCount = it[THREADS] ?: DEFAULT_THREADS,
                hashSize = it[HASH_SIZE] ?: DEFAULT_HASH_SIZE,
            )
        }

    suspend fun setEngineSettings(engineSettings: EngineSettings) =
        dataStore().edit { preferences ->
            preferences[SEARCH_DEPTH] = engineSettings.searchDepth
            preferences[QUIET_SEARCH] = engineSettings.quietSearch
            preferences[SEARCH_TIME] = engineSettings.searchTime.toInt(TimeUnit.SECONDS)
            preferences[THREADS] = engineSettings.threadCount
            preferences[HASH_SIZE] = engineSettings.hashSize
        }

    fun showBasicDebug(): Flow<Boolean> =
        dataStore().data.map { it[SHOW_DEBUG_BASIC] ?: false }

    fun showAdvancedDebug(): Flow<Boolean> =
        dataStore().data.map { it[SHOW_DEBUG_ADVANCED] ?: false }

    companion object {
        val FIRST_START = booleanPreferencesKey("first_start")

        val SHOW_COORDINATES = booleanPreferencesKey("show_coordinates")
        val SHOW_MOVES_HISTORY = booleanPreferencesKey("show_moves_history")

        val SEARCH_DEPTH = intPreferencesKey("search_depth")
        val QUIET_SEARCH = booleanPreferencesKey("quiet_search")
        val SEARCH_TIME = intPreferencesKey("search_time")
        val THREADS = intPreferencesKey("threads")
        val HASH_SIZE = intPreferencesKey("hash_size")

        val SHOW_DEBUG_BASIC = booleanPreferencesKey("show_debug_basic")
        val SHOW_DEBUG_ADVANCED = booleanPreferencesKey("show_debug_advanced")

        const val DEFAULT_SHOW_COORDINATES = true
        const val DEFAULT_MOVES_HISTORY = true
        const val DEFAULT_SEARCH_DEPTH = 6
        const val DEFAULT_QUIET_SEARCH = true
        const val DEFAULT_SEARCH_TIME = 30
        const val DEFAULT_THREADS = 1
        const val DEFAULT_HASH_SIZE = 64
    }
}
