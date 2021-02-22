package net.theluckycoder.chess

import androidx.compose.foundation.ExperimentalFoundationApi
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.lazy.GridCells
import androidx.compose.foundation.lazy.LazyVerticalGrid
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.tooling.preview.Preview
import androidx.lifecycle.asLiveData
import kotlinx.coroutines.flow.MutableStateFlow

private val isPlayerWhiteState = MutableStateFlow(true)

enum class CellState {
    NONE,
    POSSIBLE_MOVE,
    POSSIBLE_CAPTURE,
    MOVED
}

@OptIn(ExperimentalStdlibApi::class)
private val cellStates = MutableStateFlow(buildList<CellState> {
    for (i in 0 until 64)
        add(CellState.NONE)
})

@Preview
@Composable
fun PreviewMainScreen() {
    MainScreen()
}

@OptIn(ExperimentalFoundationApi::class)
@Composable
fun MainScreen() {
    val isPlayerWhite by isPlayerWhiteState.asLiveData().observeAsState(true)
    val cells by cellStates.asLiveData().observeAsState(emptyList())

    LazyVerticalGrid(
        GridCells.Fixed(64)
    ) {
        itemsIndexed(cells) { index, cell ->
            val isWhite = (index + if (isPlayerWhite) 0 else 1) % 2 == 1
            val color = when {
                isWhite -> Color.White
                else -> if (isWhite) Color.White else Color.Green
            }
            Box(Modifier.fillParentMaxSize().background(color))
        }
    }
}
