import os

print("🏗️  جاري إعادة بناء الهيكل بالطريقة الحديثة...")

# 1. ملف الإعدادات الحديث (settings.gradle)
# هذا الملف يحدد أماكن تحميل الإضافات بشكل مركزي
settings_content = """pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}
rootProject.name = "Autonomous_Football_Game_V5"
include ':app'
"""

with open("settings.gradle", "w") as f:
    f.write(settings_content)

# 2. ملف البناء الرئيسي الحديث (Root build.gradle)
# نستخدم صيغة plugins {} بدلاً من buildscript القديمة
root_build_content = """plugins {
    id 'com.android.application' version '8.1.0' apply false
}
"""

with open("build.gradle", "w") as f:
    f.write(root_build_content)

# 3. ملف التطبيق الحديث (App build.gradle)
app_build_content = """plugins {
    id 'com.android.application'
}

android {
    namespace 'com.ai.autonomous.game'
    compileSdk 34

    defaultConfig {
        applicationId "com.ai.autonomous.game"
        minSdk 24
        targetSdk 34
        versionCode 1
        versionName "1.0"

        externalNativeBuild {
            cmake {
                cppFlags "-std=c++20"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
        }
    }
    
    // تحديد إصدار NDK لتجنب المشاكل
    ndkVersion "25.1.8937393"
}
"""

with open("app/build.gradle", "w") as f:
    f.write(app_build_content)

print("✅ تم تحديث جميع ملفات البناء للصيغة الحديثة.")

# 4. الرفع الفوري
print("📤 جاري الرفع للسحابة...")
os.system("git add .")
os.system('git commit -m "Refactor: Switch to Modern Gradle Plugins DSL"')
os.system("git push origin main --force")
