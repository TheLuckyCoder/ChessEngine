package net.theluckycoder.chess.ui

import androidx.compose.material.MaterialTheme
import androidx.compose.material.darkColors
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import net.theluckycoder.chess.common.ui.AppColors

private val DarkColors = darkColors(
    primary = AppColors.Primary,
    primaryVariant = AppColors.PrimaryVariant,
    secondary = AppColors.Secondary,
    onPrimary = Color.White,
    onSecondary = Color.Black
)

@Composable
fun ChessMaterialTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colors = DarkColors,
        content = content
    )
}