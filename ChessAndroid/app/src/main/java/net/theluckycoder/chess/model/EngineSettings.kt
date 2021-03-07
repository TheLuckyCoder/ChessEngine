package net.theluckycoder.chess.model

import androidx.annotation.Keep
import kotlin.time.Duration
import kotlin.time.ExperimentalTime
import kotlin.time.minutes

@OptIn(ExperimentalTime::class)
@Keep
data class EngineSettings(
    val searchDepth: Int,
    val threadCount: Int,
    val hashSize: Int,
    val quietSearch: Boolean,
    val searchTime: Duration = 1.minutes,
)
