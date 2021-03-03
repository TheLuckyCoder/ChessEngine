package net.theluckycoder.chess.ui.preferences

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.material.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@ExperimentalMaterialApi
@Composable
fun Preference(
    item: PreferenceItem,
    summary: String? = null, // TODO
    onClick: () -> Unit = { },
    trailing: @Composable (() -> Unit)? = null
) {
    StatusWrapper(enabled = item.enabled) {
        ListItem(
            text = {
                Text(text = item.title, maxLines = if (item.singleLineTitle) 1 else Int.MAX_VALUE)
            },
            secondaryText = { Text(text = summary ?: item.summary) },
            icon = { PreferenceIcon(painter = item.icon) },
            modifier = Modifier.clickable(onClick = { if (item.enabled) onClick() }),
            trailing = trailing,
        )
    }
}

@ExperimentalMaterialApi
@Composable
fun Preference(
    item: KeyPreferenceItem<*>,
    summary: @Composable () -> Unit,
    onClick: () -> Unit = { },
    trailing: @Composable (() -> Unit)? = null
) {
    StatusWrapper(enabled = item.enabled) {
        ListItem(
            text = {
                Text(
                    text = item.title,
                    maxLines = if (item.singleLineTitle) 1 else Int.MAX_VALUE
                )
            },
            secondaryText = summary,
            icon = { PreferenceIcon(painter = item.icon) },
            modifier = Modifier.clickable(onClick = { if (item.enabled) onClick() }),
            trailing = trailing,
        )
    }
}

@Composable
private fun PreferenceIcon(painter: Painter?) {
    val iconModifier = Modifier
        .padding(8.dp)
        .size(24.dp)

    if (painter != null) {
        Icon(
            painter = painter,
            contentDescription = null,
            modifier = iconModifier
        )
    } else {
        Spacer(modifier = iconModifier)
    }
}

@Composable
fun PreferenceGroup(title: String, enabled: Boolean = true, content: @Composable () -> Unit) {
    Column {
        Box(
            contentAlignment = Alignment.CenterStart,
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 24.dp, bottom = 8.dp)
        ) {
            Text(
                text = title,
                fontSize = 14.sp,
                color = MaterialTheme.colors.secondary,
                fontWeight = FontWeight.Medium,
                modifier = Modifier.padding(start = 16.dp)
            )
        }
        StatusWrapper(enabled) {
            content()
        }
    }
}

@Composable
fun StatusWrapper(enabled: Boolean = true, content: @Composable () -> Unit) {
    CompositionLocalProvider(LocalContentAlpha provides if (enabled) ContentAlpha.high else ContentAlpha.disabled) {
        content()
    }
}
