plugins {
    id("com.android.application")

    kotlin("android")
    kotlin("kapt")
}

android {
    compileSdk = 30
    ndkVersion = "22.1.7171670"

    defaultConfig {
        applicationId = "net.theluckycoder.chess"
        minSdk = 21
        targetSdk = 30
        versionCode = 1204
        versionName = "1.2.0"
        resourceConfigurations += listOf("en")
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
//            proguardFiles getDefaultProguardFile('proguard-android.txt'), 'proguard-rules.pro'

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

    externalNativeBuild {
        cmake {
//            version = "3.18.1"
            path("CMakeLists.txt")
        }
    }
}

tasks.withType(org.jetbrains.kotlin.gradle.tasks.KotlinCompile::class).configureEach {
    kotlinOptions {
        jvmTarget = "1.8"
        useIR = true
        freeCompilerArgs = freeCompilerArgs + listOf("-Xopt-in=kotlin.RequiresOptIn")
    }
}

dependencies {
    val kotlinVersion: String by rootProject.extra
    val composeVersion: String by rootProject.extra

    // Kotlin
    implementation("org.jetbrains.kotlin:kotlin-stdlib-jdk8:$kotlinVersion")
    debugImplementation("org.jetbrains.kotlin:kotlin-reflect:$kotlinVersion")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.4.3")

    // AndroidX
    implementation("androidx.activity:activity-ktx:1.2.3")
    implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:2.3.1")
    implementation("androidx.datastore:datastore-preferences:1.0.0-beta01")

    // Compose
    implementation("androidx.compose.ui:ui:$composeVersion")
    implementation("androidx.compose.foundation:foundation:$composeVersion")
    implementation("androidx.compose.material:material:$composeVersion")
    implementation("androidx.compose.ui:ui-tooling:$composeVersion")
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:1.0.0-alpha05")
}
