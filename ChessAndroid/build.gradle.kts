buildscript {
    val kotlinVersion: String by rootProject.extra("1.5.10")
    val composeVersion: String by rootProject.extra("1.0.0-beta09")

    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath("com.android.tools.build:gradle:7.0.0-beta04")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:${kotlinVersion}")
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
    }
}

subprojects {
    // Taken from:
    // https://github.com/chrisbanes/tivi/blob/main/build.gradle
    configurations.configureEach {
        // We forcefully exclude AppCompat + MDC from any transitive dependencies.
        // This is a Compose app, so there's no need for these.
        exclude(group = "androidx.appcompat", module = "appcompat")
        exclude(group = "com.google.android.material", module = "material")
        exclude(group = "com.google.android.material", module = "material")
    }
}

plugins {
    id("com.github.ben-manes.versions") version "0.39.0"
}

tasks.named("dependencyUpdates", com.github.benmanes.gradle.versions.updates.DependencyUpdatesTask::class.java).configure {
    rejectVersionIf {
        candidate.version.contains("alpha") && !this@rejectVersionIf.currentVersion.contains("alpha")
    }
}

tasks.register("clean", Delete::class) {
    delete(rootProject.buildDir)
}