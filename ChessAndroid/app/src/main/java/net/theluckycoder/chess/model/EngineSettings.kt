package net.theluckycoder.chess.model

data class EngineSettings(
    val searchDepth: Int,
    val threadCount: Int,
    val cacheSize: Int,
    val doQuietSearch: Boolean
) {

    companion object {
        private val MAX_THREAD_COUNT = Runtime.getRuntime().availableProcessors()

        fun create(
            searchDepth: Int,
            threadCount: Int,
            cacheSize: Int,
            doQuietSearch: Boolean
        ): EngineSettings {
            val threads = when {
                threadCount > MAX_THREAD_COUNT -> MAX_THREAD_COUNT
                threadCount < 1 -> MAX_THREAD_COUNT - 1
                else -> threadCount
            }
            return EngineSettings(
                if (searchDepth <= 0) 1 else searchDepth,
                threads,
                cacheSize,
                doQuietSearch
            )
        }
    }
}
