package net.theluckycoder.chess.model

import androidx.annotation.Keep
import kotlin.time.Duration
import kotlin.time.ExperimentalTime
import kotlin.time.milliseconds

@OptIn(ExperimentalTime::class)
@Keep
data class SearchOptions(
    val searchDepth: Int,
    val threadCount: Int,
    val searchTime: Duration,
    val hashSize: Int,
    val quietSearch: Boolean,
) {
    @Keep
    constructor(
        searchDepth: Int,
        threadCount: Int,
        searchTime: Long,
        hashSize: Int,
        quietSearch: Boolean,
    ) : this(searchDepth, threadCount, searchTime.milliseconds, hashSize, quietSearch)

    companion object {

        @JvmStatic
        external fun getNativeSearchOptions(): SearchOptions

        @OptIn(ExperimentalTime::class)
        fun setNativeSearchOptions(options: SearchOptions) = setNativeSearchOptions(
            options.searchDepth,
            options.quietSearch,
            options.threadCount,
            options.hashSize,
            options.searchTime.toLongMilliseconds(),
        )

        @JvmStatic
        private external fun setNativeSearchOptions(
            searchDepth: Int, quietSearch: Boolean,
            threadCount: Int, hashSizeMb: Int,
            searchTime: Long,
        )
    }
}
