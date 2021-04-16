package net.theluckycoder.chess.model

import androidx.annotation.Keep

@Keep
enum class GameState {
    NONE,
    WINNER_WHITE,
    WINNER_BLACK,
    DRAW,
    WHITE_IN_CHECK,
    BLACK_IN_CHECK,
    INVALID,
}
