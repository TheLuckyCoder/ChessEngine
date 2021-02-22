package net.theluckycoder.resourcepackconverter.ui

import androidx.compose.foundation.ExperimentalFoundationApi
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.lazy.GridCells
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyVerticalGrid
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.material.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.tooling.preview.Preview
import androidx.lifecycle.asLiveData
import kotlinx.coroutines.flow.MutableStateFlow

private val isPlayerWhiteState = MutableStateFlow(true)

data class Cell(
    val index: Int,
    val state: State
) {

    enum class State {
        NONE,
        POSSIBLE_MOVE,
        POSSIBLE_CAPTURE,
        MOVED
    }
}

@OptIn(ExperimentalStdlibApi::class)
private val cellStates = MutableStateFlow(buildList<Cell> {
    for (i in 0 until 64)
        add(Cell(i, Cell.State.NONE))
})

@Preview
@Composable
fun PreviewMainScreen() {
    MainScreen()
}

@OptIn(ExperimentalFoundationApi::class)
@Composable
fun MainScreen() = Box(Modifier.fillMaxSize()) {
    val isPlayerWhite by isPlayerWhiteState.asLiveData().observeAsState(true)
    val cells by cellStates.asLiveData().observeAsState(emptyList())

    LazyVerticalGrid(
        GridCells.Fixed(8),
        modifier = Modifier.fillMaxWidth()
    ) {
        itemsIndexed(cells) { index, cell ->
            val isWhite = (index + if (isPlayerWhite) 0 else 1) % 2 == 1
            val color = when {
                isWhite -> Color.White
                else -> if (isWhite) Color.White else Color.Green
            }
            Surface(modifier = Modifier.fillParentMaxSize().background(color)) {

            }
        }
    }
}
