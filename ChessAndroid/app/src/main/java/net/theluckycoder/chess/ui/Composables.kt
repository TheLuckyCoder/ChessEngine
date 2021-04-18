package net.theluckycoder.chess.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.Icon
import androidx.compose.material.IconToggleButton
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.MutableState
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import net.theluckycoder.chess.R

@Composable
fun AlertDialogTitle(text: String) {
    Text(
        text = text,
        fontSize = 18.sp,
        fontWeight = FontWeight.Bold
    )
}

@Composable
fun ChooseSidesToggle(
    modifier: Modifier = Modifier,
    sidesToggleIndex: MutableState<Int>,
) {
    class Side(val painterRes: Int, val backgroundColorRes: Int, val contentDescriptionRes: Int)

    val sides = listOf(
        Side(R.drawable.w_king, R.color.side_white, R.string.side_white),
        Side(R.drawable.b_king, R.color.side_black, R.string.side_black),
        Side(R.drawable.side_random, R.color.side_random, R.string.side_random),
    )

    Row(
        modifier = modifier
            .fillMaxWidth()
            .padding(8.dp)
            .clip(RoundedCornerShape(8.dp))
    ) {
        sides.forEachIndexed { index, side ->
            val backgroundColor = if (sidesToggleIndex.value == index)
                MaterialTheme.colors.primary.copy(alpha = 0.5f)
            else
                colorResource(id = side.backgroundColorRes)

            IconToggleButton(
                modifier = Modifier
                    .background(backgroundColor)
                    .weight(1f)
                    .padding(4.dp),
                checked = sidesToggleIndex.value == index,
                onCheckedChange = { sidesToggleIndex.value = index }
            ) {
                Icon(
                    modifier = Modifier.size(54.dp),
                    painter = painterResource(id = side.painterRes),
                    tint = Color.Unspecified,
                    contentDescription = stringResource(id = side.contentDescriptionRes),
                )
            }
        }
    }
}

