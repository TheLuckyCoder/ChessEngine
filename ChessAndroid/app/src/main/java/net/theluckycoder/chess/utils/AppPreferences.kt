package net.theluckycoder.chess.utils

import android.content.Context
import androidx.preference.PreferenceManager
import net.theluckycoder.chess.R
import net.theluckycoder.chess.model.Settings
import net.theluckycoder.chess.model.BoardAppearance

class AppPreferences(private val context: Context) {

    companion object {
        const val KEY_FIRST_START = "key_first_start"

        const val KEY_TILE_WHITE = "key_tile_white"
        const val KEY_TILE_BLACK = "key_tile_black"
        const val KEY_TILE_POSSIBLE = "key_tile_possible"
        const val KEY_TILE_LAST_MOVED = "key_tile_last_moved"
        const val KEY_KING_IN_CHECK = "key_king_in_check"
        const val KEY_SHOW_COORDS = "key_show_coordinates"
        const val KEY_RESET_APPEARANCE = "key_reset_appearance"

        const val KEY_SEARCH_DEPTH = "key_search_depth"
        const val KEY_THREAD_COUNT = "key_thread_count"
        const val KEY_CACHE_SIZE = "key_cache_size"
        const val KEY_QUIET_SEARCH = "key_quiet_search"
        const val KEY_DIFFICULTY_LEVEL = "key_difficulty_level"

        const val KEY_DEBUG_INFO_BASIC = "key_show_debug_basic"
        const val KEY_DEBUG_INFO_ADVANCED = "key_show_debug_advanced"

        const val KEY_PERFT_TEST = "key_perft_test"
        const val KEY_EVALUATION_TEST = "key_evaluation_test"
    }

    private val manager by lazy { PreferenceManager.getDefaultSharedPreferences(context) }

    var firstStart: Boolean
        get() = manager.getBoolean(KEY_FIRST_START, true)
        set(value) = manager.edit().putBoolean(KEY_FIRST_START, value).apply()

    var whiteTileColor: Int
        get() = manager.getInt(KEY_TILE_WHITE, getColor(context, R.color.tile_white))
        set(value) = manager.edit().putInt(KEY_TILE_WHITE, value).apply()

    var blackTileColor: Int
        get() = manager.getInt(KEY_TILE_BLACK, getColor(context, R.color.tile_black))
        set(value) = manager.edit().putInt(KEY_TILE_BLACK, value).apply()

    var possibleMoveColor: Int
        get() = manager.getInt(KEY_TILE_POSSIBLE, getColor(context, R.color.tile_possible))
        set(value) = manager.edit().putInt(KEY_TILE_POSSIBLE, value).apply()

    var lastMovedTileColor: Int
        get() = manager.getInt(KEY_TILE_LAST_MOVED, getColor(context, R.color.tile_last_moved))
        set(value) = manager.edit().putInt(KEY_TILE_LAST_MOVED, value).apply()

    var inCheckColor: Int
        get() = manager.getInt(KEY_KING_IN_CHECK, getColor(context, R.color.king_in_check))
        set(value) = manager.edit().putInt(KEY_KING_IN_CHECK, value).apply()

    var showCoordinates: Boolean
        get() = manager.getBoolean(KEY_SHOW_COORDS, true)
        set(value) = manager.edit().putBoolean(KEY_SHOW_COORDS, value).apply()

    val boardAppearance: BoardAppearance
        get() = BoardAppearance(
            whiteTileColor,
            blackTileColor,
            possibleMoveColor,
            lastMovedTileColor,
            inCheckColor,
            showCoordinates
        )

    var settings: Settings
        get() = Settings.create(
            searchDepth = manager.getInt(KEY_SEARCH_DEPTH, 4),
            threadCount = manager.getInt(
                KEY_THREAD_COUNT,
                Runtime.getRuntime().availableProcessors() - 1
            ),
            cacheSize = manager.getString(
                KEY_CACHE_SIZE,
                null
            )?.toIntOrNull() ?: 100,
            doQuietSearch = manager.getBoolean(KEY_QUIET_SEARCH, true)
        )
        set(value) {
            manager.edit()
                .putInt(KEY_SEARCH_DEPTH, value.searchDepth)
                .putInt(KEY_THREAD_COUNT, value.threadCount)
                .putString(KEY_CACHE_SIZE, value.cacheSize.toString())
                .putBoolean(KEY_QUIET_SEARCH, value.doQuietSearch)
                .apply()
        }

    var difficultyLevel: Int
        get() = manager.getInt(KEY_DIFFICULTY_LEVEL, 0)
        set(value) = manager.edit().putInt(KEY_DIFFICULTY_LEVEL, value).apply()

    val basicDebugInfo: Boolean
        get() = manager.getBoolean(KEY_DEBUG_INFO_BASIC, false)

    val advancedDebugInfo: Boolean
        get() = manager.getBoolean(KEY_DEBUG_INFO_ADVANCED, false)
}