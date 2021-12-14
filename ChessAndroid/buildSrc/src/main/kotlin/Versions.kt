object Versions {
    object App {
        private const val major = 1
        private const val minor = 2
        private const val patch = 2
        private const val build = 1

        const val code = major * 1000 + minor * 100 + patch * 10 + build
        const val name = "$major.$minor.$patch"
    }

    object Sdk {
        const val min = 21
        const val wearOsMin = 25
        const val compile = 31
        const val target = 31
    }

    const val kotlin = "1.6.0"
    const val kotlinCoroutines = "1.5.2"
    const val compose = "1.1.0-beta04"
    const val voyager = "1.0.0-beta13"
}
