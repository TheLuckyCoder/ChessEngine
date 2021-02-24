package net.theluckycoder.chess.model

data class Tile(
    val index: Int,
    val state: State
) {

    sealed class State {
        object None : State()
        class PossibleMove(val move: Move) : State()
        object Selected : State()
        object Moved : State()
    }
}