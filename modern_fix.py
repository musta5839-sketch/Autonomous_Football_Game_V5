import os

print("🏗️  جاري تحديث هيكل المشروع إلى النظام الحديث (Modern DSL)...")

# 1. تحديث settings.gradle (المسؤول عن جلب الإضافات)
# هذا الملف يخبر جرادل أين يجد الأدوات
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

# 2. تحديث build.gradle الرئيسي (تنظيف التضاربات القديمة)
# نحدد النسخة هنا مرة واحدة فقط
root_build_content = """plugins {
    id 'com.android.application' version '8.1.0' apply false
}
"""

with open("build.gradle", "w") as f:
    f.write(root_build_content)

# 3. تحديث app/build.gradle (استخدام الصيغة الحديثة)
# نطلب الأداة بدون ذكر النسخة لأننا ذكرناها في الملف الرئيسي
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
    
    // تحديد إصدار NDK لتفادي أي خطأ مستقبلي
    ndkVersion "25.1.8937393"
}
"""

with open("app/build.gradle", "w") as f:
    f.write(app_build_content)

print("✅ تم تحديث ملفات البناء بنجاح.")

# 4. الرفع وتفعيل البناء
print("📤 جاري الرفع...")
os.system("git add .")
os.system('git commit -m "Refactor: Switch entire project to Modern Gradle Plugins DSL"')
os.system("git push origin main --force")
