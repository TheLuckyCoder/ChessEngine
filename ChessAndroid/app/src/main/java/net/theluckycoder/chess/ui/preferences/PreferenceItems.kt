package net.theluckycoder.chess.ui.preferences

import androidx.compose.ui.graphics.painter.Painter
import androidx.datastore.preferences.core.Preferences
import kotlin.math.roundToInt

interface BasePreferenceItem

interface PreferenceItem<T> : BasePreferenceItem {
    val title: String
    val summary: String
    val prefKey: Preferences.Key<T>
    val icon: Painter
    val singleLineTitle: Boolean
    val enabled: Boolean
}

class SwitchPreferenceItem(
    override val title: String,
    override val summary: String,
    override val prefKey: Preferences.Key<Boolean>,
    override val icon: Painter,
    override val singleLineTitle: Boolean = false,
    override val enabled: Boolean = true,
    val defaultValue: Boolean = false,
) : PreferenceItem<Boolean>

abstract class SeekbarBasePreferenceItem<T : Number>(
    final override val title: String,
    final override val summary: String,
    final override val prefKey: Preferences.Key<T>,
    final override val icon: Painter,
    final override val singleLineTitle: Boolean,
    final override val enabled: Boolean,
    val defaultValue: T,
    val valueRange: ClosedFloatingPointRange<Float>,
    val steps: Int,
    val valueRepresentation: (T) -> String
) : PreferenceItem<T>

class SeekbarIntPreferenceItem(
    title: String,
    summary: String,
    prefKey: Preferences.Key<Int>,
    icon: Painter,
    singleLineTitle: Boolean = false,
    enabled: Boolean = true,
    defaultValue: Int = 0,
    valueRange: IntRange = 0..100,
    steps: Int = 0,
    valueRepresentation: (Int) -> String = { it.toString() }
) : SeekbarBasePreferenceItem<Int>(
    title = title,
    summary = summary,
    prefKey = prefKey,
    icon = icon,
    singleLineTitle = singleLineTitle,
    enabled = enabled,
    defaultValue = defaultValue,
    valueRange = valueRange.first.toFloat()..valueRange.last.toFloat(),
    steps = steps,
    valueRepresentation = valueRepresentation
)

class SeekbarFloatPreferenceItem(
    title: String,
    summary: String,
    prefKey: Preferences.Key<Float>,
    icon: Painter,
    singleLineTitle: Boolean = false,
    enabled: Boolean = true,
    defaultValue: Float = 0f,
    valueRange: ClosedFloatingPointRange<Float> = 0f..1f,
    steps: Int = 0,
    valueRepresentation: (Float) -> String = { it.roundToInt().toString() }
) : SeekbarBasePreferenceItem<Float>(
    title = title,
    summary = summary,
    prefKey = prefKey,
    icon = icon,
    singleLineTitle = singleLineTitle,
    enabled = enabled,
    defaultValue = defaultValue,
    valueRange = valueRange,
    steps = steps,
    valueRepresentation = valueRepresentation
)

class PreferenceGroupItem(
    val title: String,
    val enabled: Boolean = true,
    val items: List<PreferenceItem<*>>
) : BasePreferenceItem
