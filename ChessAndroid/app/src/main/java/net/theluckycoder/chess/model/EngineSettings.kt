package net.theluckycoder.chess.model

import androidx.annotation.Keep

@Keep
data class EngineSettings(
    val searchDepth: Int,
    val threadCount: Int,
    val hashSize: Int,
    val quietSearch: Boolean
)
