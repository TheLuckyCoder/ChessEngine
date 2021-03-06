package net.theluckycoder.chess.utils

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.intPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.map
import net.theluckycoder.chess.model.EngineSettings

val Context.settingsDataStore: DataStore<Preferences> by preferencesDataStore(
    name = "settings",
)

class SettingsDataStore(private val context: Context) {

    private fun dataStore() = context.settingsDataStore

    fun firstStart(): Flow<Boolean> =
        dataStore().data.map { it[FIRST_START] ?: true }

    suspend fun setFirstStart(value: Boolean) = dataStore().edit { preferences ->
        preferences[FIRST_START] = value
    }

    suspend fun setDifficultyLevel(value: Int) = dataStore().edit { preferences ->
        preferences[SEARCH_DEPTH] = if (value == 0 || value == 1) value + 2 else value + 3
        preferences[QUIET_SEARCH] = value != 0
    }

    fun searchDepth(): Flow<Int> =
        dataStore().data.map { it[SEARCH_DEPTH] ?: DEFAULT_SEARCH_DEPTH }

    fun quietSearch(): Flow<Boolean> =
        dataStore().data.map { it[QUIET_SEARCH] ?: DEFAULT_QUIET_SEARCH }

    fun threads(): Flow<Int> =
        dataStore().data.map { it[THREADS] ?: DEFAULT_THREADS }

    fun hashSize(): Flow<Int> =
        dataStore().data.map { it[HASH_SIZE] ?: DEFAULT_HASH_SIZE }

    suspend fun getEngineSettings() = EngineSettings(
        searchDepth = searchDepth().first(),
        quietSearch = quietSearch().first(),
        threadCount = threads().first(),
        hashSize = hashSize().first(),
    )

    suspend fun setEngineSettings(engineSettings: EngineSettings) =
        dataStore().edit { preferences ->
            preferences[SEARCH_DEPTH] = engineSettings.searchDepth
            preferences[QUIET_SEARCH] = engineSettings.quietSearch
            preferences[THREADS] = engineSettings.threadCount
            preferences[HASH_SIZE] = engineSettings.hashSize
        }

    fun showBasicDebug(): Flow<Boolean> =
        dataStore().data.map { it[SHOW_DEBUG_BASIC] ?: false }

    fun showAdvancedDebug(): Flow<Boolean> =
        dataStore().data.map { it[SHOW_DEBUG_ADVANCED] ?: false }

    companion object {
        val FIRST_START = booleanPreferencesKey("first_start")

        val SEARCH_DEPTH = intPreferencesKey("search_depth")
        val QUIET_SEARCH = booleanPreferencesKey("quiet_search")
        val THREADS = intPreferencesKey("threads")
        val HASH_SIZE = intPreferencesKey("hash_size")

        val SHOW_DEBUG_BASIC = booleanPreferencesKey("show_debug_basic")
        val SHOW_DEBUG_ADVANCED = booleanPreferencesKey("show_debug_advanced")

        const val DEFAULT_SEARCH_DEPTH = 6
        const val DEFAULT_QUIET_SEARCH = true
        const val DEFAULT_THREADS = 1
        const val DEFAULT_HASH_SIZE = 64
    }
}
