package net.theluckycoder.chess.model

import androidx.annotation.Keep
import kotlin.time.Duration
import kotlin.time.ExperimentalTime

@OptIn(ExperimentalTime::class)
@Keep
data class EngineSettings(
    val searchDepth: Int,
    val threadCount: Int,
    val searchTime: Duration,
    val hashSize: Int,
    val quietSearch: Boolean,
)
