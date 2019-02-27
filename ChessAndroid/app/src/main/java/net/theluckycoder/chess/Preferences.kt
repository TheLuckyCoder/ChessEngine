package net.theluckycoder.chess

import android.content.Context
import android.preference.PreferenceManager

class Preferences(private val context: Context) {

    companion object {
        const val KEY_TILE_WHITE = "key_tile_white"
        const val KEY_TILE_BLACK = "key_tile_black"
        const val KEY_TILE_POSSIBLE = "key_tile_possible"
        const val KEY_TILE_SELECTED = "key_tile_selected"
        const val KEY_TILE_LAST_MOVED = "key_tile_last_moved"
        const val KEY_RESET_TILE_COLORS = "key_reset_tile_colors"

        const val KEY_DEPTH = "key_depth"
        const val KEY_THREADS = "key_threads"
        const val KEY_DEBUG_INFO = "key_debug_info"
    }

    private val manager
        get() = PreferenceManager.getDefaultSharedPreferences(context)!!

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

    var settings
        get() = Settings(
            manager.getString(KEY_DEPTH, null)?.toIntOrNull() ?: 0,
            manager.getString(KEY_THREADS, null)?.toIntOrNull() ?: 0
        )
        set(value) {
            manager.edit()
                .putString(KEY_DEPTH, value.baseSearchDepth.toString())
                .putString(KEY_THREADS, value.threadCount.toString())
                .apply()
        }

    val debugInfo
        get() = manager.getBoolean(KEY_DEBUG_INFO, false)
}