plugins {
    id("com.android.application")

    kotlin("android")
}

android {
    compileSdk = 31
    ndkVersion = "23.0.7599858"

    defaultConfig {
        applicationId = "net.theluckycoder.chess.wearos"
        minSdk = 28
        targetSdk = 31
        versionCode = 1
        versionName = "1.0"
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
}

dependencies {
    implementation("androidx.core:core-ktx:1.6.0")
    implementation("androidx.wear:wear:1.1.0")

    implementation("com.google.android.support:wearable:2.8.1")
    compileOnly("com.google.android.wearable:wearable:2.8.1")

    implementation("androidx.wear.compose:compose-foundation:1.0.0-alpha04")

    // For Wear Material Design UX guidelines and specifications
    implementation("androidx.wear.compose:compose-material:1.0.0-alpha04")
}