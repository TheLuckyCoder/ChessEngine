object Versions {
    object App {
        private const val major = 1
        private const val minor = 3
        private const val patch = 0
        private const val build = 0

        const val code = major * 1000 + minor * 100 + patch * 10 + build
        const val name = "$major.$minor.$patch"
    }

    object Sdk {
        const val min = 21
        const val wearOsMin = 25
        const val compile = 33
        const val target = 33
    }

    const val kotlin = "1.7.21"
    const val kotlinCoroutines = "1.6.4"
    const val compose = "1.3.1"
    const val composeCompiler = "1.4.0-alpha02"
}
