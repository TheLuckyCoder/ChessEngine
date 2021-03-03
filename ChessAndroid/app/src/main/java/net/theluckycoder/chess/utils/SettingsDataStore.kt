package net.theluckycoder.chess.utils

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.*
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.*
import net.theluckycoder.chess.model.EngineSettings

val Context.settingsDataStore: DataStore<Preferences> by preferencesDataStore(
    name = "settings",
)

class SettingsDataStore(private val context: Context) {

    private fun dataStore() = context.settingsDataStore

    fun difficultyLevel(): Flow<Int> =
        dataStore().data.map { it[DIFFICULTY_LEVEL] ?: 1 }

    suspend fun setDifficultyLevel(value: Int) = dataStore().edit { preferences ->
        preferences[DIFFICULTY_LEVEL] = value
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

    companion object {
        val DIFFICULTY_LEVEL = intPreferencesKey("key_difficulty_level")
        val SEARCH_DEPTH = intPreferencesKey("key_search_depth")
        val QUIET_SEARCH = booleanPreferencesKey("key_quiet_search")
        val THREADS = intPreferencesKey("key_threads")
        val HASH_SIZE = intPreferencesKey("key_hash_size")

        val SHOW_DEBUG_BASIC = booleanPreferencesKey("key_show_debug_basic")
        val SHOW_DEBUG_ADVANCED = booleanPreferencesKey("key_show_debug_advanced")

        const val DEFAULT_SEARCH_DEPTH = 6
        const val DEFAULT_QUIET_SEARCH = true
        val DEFAULT_THREADS = (Runtime.getRuntime().availableProcessors() / 2).coerceAtLeast(1)
        const val DEFAULT_HASH_SIZE = 64
    }
}
