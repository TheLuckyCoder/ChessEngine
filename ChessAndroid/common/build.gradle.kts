plugins {
    id("com.android.library")

    kotlin("android")
}

android {
    namespace = "net.theluckycoder.chess.common"
    compileSdk = Versions.Sdk.compile
    ndkVersion = "25.1.8937393"

    defaultConfig {
        minSdk = Versions.Sdk.min
        targetSdk = Versions.Sdk.target
        resourceConfigurations += listOf("en")

        consumerProguardFiles("consumer-rules.pro")
    }

    buildFeatures.compose = true

    composeOptions {
        kotlinCompilerExtensionVersion = Versions.composeCompiler
    }

    buildTypes {
        release {
            externalNativeBuild {
                cmake {
                    arguments += listOf("-DCMAKE_BUILD_TYPE=Release")
                }
            }
        }
    }

    externalNativeBuild {
        cmake {
            version = "3.22.1"
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
    val composeVersion = Versions.compose

    // Kotlin
    kotlin("stdlib-jdk8", Versions.kotlin)
    debugApi("org.jetbrains.kotlin:kotlin-reflect:${Versions.kotlin}")
    api("org.jetbrains.kotlinx:kotlinx-coroutines-android:${Versions.kotlinCoroutines}")

    // AndroidX
    api("androidx.lifecycle:lifecycle-viewmodel-ktx:2.5.1")
    api("androidx.datastore:datastore-preferences:1.0.0")
    api("androidx.lifecycle:lifecycle-viewmodel-compose:2.5.1")

    // Compose
    api("androidx.activity:activity-compose:1.6.1")
    api("androidx.compose.ui:ui:$composeVersion")
    api("androidx.compose.foundation:foundation:$composeVersion")
    implementation("androidx.compose.material:material:$composeVersion")
    api("androidx.compose.ui:ui-tooling-preview:$composeVersion")
    debugApi("androidx.compose.ui:ui-tooling:$composeVersion")
    api("androidx.compose.animation:animation-graphics:$composeVersion")
    api("androidx.lifecycle:lifecycle-viewmodel-compose:2.5.1")
}
