plugins {
    id("com.android.application")

    kotlin("android")
}

android {
    compileSdk = Versions.Sdk.compile

    defaultConfig {
        applicationId = "net.theluckycoder.chess"
        minSdk = Versions.Sdk.wearOsMin
        targetSdk = Versions.Sdk.target
        versionCode = Versions.App.code
        versionName = Versions.App.name
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
        kotlinCompilerExtensionVersion = Versions.compose
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
    implementation(project(path = ":common"))

    implementation("androidx.wear.compose:compose-foundation:1.0.0-alpha10")
    implementation("androidx.wear.compose:compose-material:1.0.0-alpha10")
}
