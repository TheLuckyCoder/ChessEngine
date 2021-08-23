plugins {
    id("com.android.application")

    kotlin("android")
}

android {
    compileSdk = 31
    ndkVersion = "23.0.7599858"

    defaultConfig {
        applicationId = "net.theluckycoder.chess"
        minSdk = 21
        targetSdk = 30
        versionCode = 1210
        versionName = "1.2.1"
        resourceConfigurations += listOf("en")
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")

            packagingOptions {
                resources {
                    excludes.add("DebugProbesKt.bin")
                }
            }
        }
    }

    buildFeatures.compose = true

    composeOptions {
        kotlinCompilerExtensionVersion = rootProject.extra["composeVersion"] as String
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

    implementation(project(path = ":common"))

    // Kotlin
    kotlin("stdlib-jdk8", kotlinVersion)
}
