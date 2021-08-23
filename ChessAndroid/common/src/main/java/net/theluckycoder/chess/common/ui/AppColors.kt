package net.theluckycoder.chess.common.ui

import androidx.compose.material.Colors
import androidx.compose.material.MaterialTheme
import androidx.compose.material.darkColors
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

private object AppColors {
    private val PRIMARY = Color(0xFF1a822c)
    private val PRIMARY_VARIANT = Color(0xFF005400)
    private val SECONDARY = Color(0xFFffea00)

    private val DarkColors = darkColors(
        primary = PRIMARY,
        primaryVariant = PRIMARY_VARIANT,
        secondary = SECONDARY,
        onPrimary = Color.White,
        onSecondary = Color.Black
    )

    @Composable
    fun appMaterialColors(): Colors = DarkColors
}

@Composable
fun ChessMaterialTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colors = AppColors.appMaterialColors(),
        content = content
    )
}