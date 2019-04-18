package net.theluckycoder.chess

import android.content.Context
import android.content.SharedPreferences
import android.preference.PreferenceManager

class Preferences(private val context: Context) {

    companion object {
        const val KEY_FIRST_START = "key_first_start"

        const val KEY_TILE_WHITE = "key_tile_white"
        const val KEY_TILE_BLACK = "key_tile_black"
        const val KEY_TILE_POSSIBLE = "key_tile_possible"
        const val KEY_TILE_SELECTED = "key_tile_selected"
        const val KEY_TILE_LAST_MOVED = "key_tile_last_moved"
        const val KEY_KING_IN_CHESS = "key_king_in_chess"
        const val KEY_RESET_TILE_COLORS = "key_reset_tile_colors"

        const val KEY_DEPTH = "key_depth"
        const val KEY_THREADS = "key_threads"
        const val KEY_CACHE_SIZE = "key_cache_size"
        const val KEY_QUIET_SEARCH = "key_quiet_search"
        const val KEY_DEBUG_INFO = "key_debug_info"
    }

    private val manager
        get(): SharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)

    var firstStart
        get() = manager.getBoolean(KEY_FIRST_START, true)
        set(value) = manager.edit().putBoolean(KEY_FIRST_START, value).apply()

    var whiteTileColor
        get() = manager.getInt(KEY_TILE_WHITE, getColor(context, R.color.tile_white))
        set(value) = manager.edit().putInt(KEY_TILE_WHITE, value).apply()

    var blackTileColor
        get() = manager.getInt(KEY_TILE_BLACK, getColor(context, R.color.tile_black))
        set(value) = manager.edit().putInt(KEY_TILE_BLACK, value).apply()

    var possibleTileColor
        get() = manager.getInt(KEY_TILE_POSSIBLE, getColor(context, R.color.tile_possible))
        set(value) = manager.edit().putInt(KEY_TILE_POSSIBLE, value).apply()

    var selectedTileColor
        get() = manager.getInt(KEY_TILE_SELECTED, getColor(context, R.color.tile_selected))
        set(value) = manager.edit().putInt(KEY_TILE_SELECTED, value).apply()

    var lastMovedTileColor
        get() = manager.getInt(KEY_TILE_LAST_MOVED, getColor(context, R.color.tile_last_moved))
        set(value) = manager.edit().putInt(KEY_TILE_LAST_MOVED, value).apply()

    var kingInChessColor
        get() = manager.getInt(KEY_KING_IN_CHESS, getColor(context, R.color.king_in_chess))
        set(value) = manager.edit().putInt(KEY_KING_IN_CHESS, value).apply()

    var settings
        get() = Settings(
            baseSearchDepth = manager.getString(KEY_DEPTH, null)?.toIntOrNull() ?: 4,
            threadCount = manager.getString(KEY_THREADS, null)?.toIntOrNull()
                ?: Runtime.getRuntime().availableProcessors() - 1,
            cacheSize = manager.getString(KEY_CACHE_SIZE, null)?.toIntOrNull() ?: 200,
            performQuiescenceSearch = manager.getBoolean(KEY_QUIET_SEARCH, true)
        )
        set(value) {
            manager.edit()
                .putString(KEY_DEPTH, value.baseSearchDepth.toString())
                .putString(KEY_THREADS, value.threadCount.toString())
                .putString(KEY_CACHE_SIZE, value.cacheSize.toString())
                .putBoolean(KEY_QUIET_SEARCH, value.performQuiescenceSearch)
                .apply()
        }

    val debugInfo
        get() = manager.getBoolean(KEY_DEBUG_INFO, false)
}