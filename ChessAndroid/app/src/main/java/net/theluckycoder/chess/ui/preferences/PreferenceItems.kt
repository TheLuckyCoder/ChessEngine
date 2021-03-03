package net.theluckycoder.chess.ui.preferences

import androidx.compose.ui.graphics.painter.Painter
import androidx.datastore.preferences.core.Preferences
import kotlin.math.roundToInt

interface BasePreferenceItem {
    val title: String
    val enabled: Boolean
}

class PreferenceGroupItem(
    override val title: String,
    override val enabled: Boolean = true,
    val items: List<PreferenceItem>
) : BasePreferenceItem

sealed class PreferenceItem : BasePreferenceItem {
    abstract override val title: String
    abstract val summary: String
    abstract val icon: Painter?
    abstract val singleLineTitle: Boolean
    abstract override val enabled: Boolean
}

class EmptyPreferenceItem(
    override val title: String,
    override val summary: String,
    override val icon: Painter? = null,
    override val singleLineTitle: Boolean = false,
    override val enabled: Boolean = true,
    val onClick: () -> Unit = { },
) : PreferenceItem()

sealed class KeyPreferenceItem<T> : PreferenceItem() {
    abstract val prefKey: Preferences.Key<T>
}

class SwitchPreferenceItem(
    override val title: String,
    override val summary: String,
    override val prefKey: Preferences.Key<Boolean>,
    override val icon: Painter? = null,
    override val singleLineTitle: Boolean = false,
    override val enabled: Boolean = true,
    val defaultValue: Boolean = false,
) : KeyPreferenceItem<Boolean>()

sealed class SeekbarBasePreferenceItem<T : Number>(
    final override val title: String,
    final override val summary: String,
    final override val prefKey: Preferences.Key<T>,
    final override val icon: Painter?,
    final override val singleLineTitle: Boolean,
    final override val enabled: Boolean,
    val defaultValue: T,
    val valueRange: ClosedFloatingPointRange<Float>,
    val steps: Int,
    val valueRepresentation: (T) -> String
) : KeyPreferenceItem<T>()

class SeekbarIntPreferenceItem(
    title: String,
    summary: String,
    prefKey: Preferences.Key<Int>,
    icon: Painter? = null,
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
    icon: Painter? = null,
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
