plugins {
    id("com.android.library")

    kotlin("android")
}

android {
    compileSdk = 31
    ndkVersion = "23.0.7599858"

    defaultConfig {
        minSdk = 21
        targetSdk = 30
        resourceConfigurations += listOf("en")

        consumerProguardFiles("consumer-rules.pro")
    }

    buildFeatures.compose = true

    composeOptions {
        kotlinCompilerExtensionVersion = rootProject.extra["composeVersion"] as String
    }

    externalNativeBuild {
        cmake {
            version = "3.18.1"
            path("CMakeLists.txt")
        }
    }
}

tasks.withType(org.jetbrains.kotlin.gradle.tasks.KotlinCompile::class).configureEach {
    kotlinOptions {
        freeCompilerArgs = listOf(
            "-Xopt-in=kotlin.RequiresOptIn",
        )
    }
}

dependencies {
    val kotlinVersion: String by rootProject.extra
    val composeVersion: String by rootProject.extra

    // Kotlin
    kotlin("stdlib-jdk8", kotlinVersion)
    debugApi("org.jetbrains.kotlin:kotlin-reflect:$kotlinVersion")
    api("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.5.1")

    // AndroidX
    api("androidx.activity:activity-ktx:1.3.1")
    api("androidx.lifecycle:lifecycle-viewmodel-ktx:2.3.1")
    api("androidx.datastore:datastore-preferences:1.0.0")

    // Compose
    api("androidx.compose.ui:ui:$composeVersion")
    api("androidx.compose.foundation:foundation:$composeVersion")
    api("androidx.compose.material:material:$composeVersion")
    api("androidx.compose.ui:ui-tooling-preview:$composeVersion")
    debugApi("androidx.compose.ui:ui-tooling:$composeVersion")
    api("androidx.compose.animation:animation-graphics:$composeVersion")
    api("androidx.lifecycle:lifecycle-viewmodel-compose:1.0.0-alpha07")
}
