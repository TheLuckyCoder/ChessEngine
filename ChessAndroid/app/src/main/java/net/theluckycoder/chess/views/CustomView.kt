package net.theluckycoder.chess.views

import android.content.Context
import android.view.View

abstract class CustomView(context: Context) : View(context) {

    interface SimpleClickListener {
        fun onClick(view: CustomView)
    }

    init {
        isSoundEffectsEnabled = false
    }
}
