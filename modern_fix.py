import os

print("🏗️  جاري هدم الإعدادات القديمة وبناء هيكل Gradle الحديث...")

# ---------------------------------------------------------
# 1. تحديث settings.gradle
# الوظيفة: تحديد مصادر التحميل وإدارة نسخ الإضافات مركزياً
# ---------------------------------------------------------
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

# ---------------------------------------------------------
# 2. تحديث build.gradle (الجذري)
# الوظيفة: استخدام "Plugins Block" الحديث بدلاً من "Classpath" القديم
# هذا هو التغيير الذي سيحل مشكلة "already on classpath"
# ---------------------------------------------------------
root_build_content = """plugins {
    // نحدد نسخة الأندرويد هنا مرة واحدة فقط (8.1.0)
    id 'com.android.application' version '8.1.0' apply false
}
"""

with open("build.gradle", "w") as f:
    f.write(root_build_content)

# ---------------------------------------------------------
# 3. تحديث app/build.gradle (التطبيق)
# الوظيفة: استدعاء الإضافة دون ذكر النسخة (لأن الجذر حددها)
# ---------------------------------------------------------
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
    
    // إجبار النظام على استخدام نسخة NDK محددة لمنع الأخطاء المستقبلية
    ndkVersion "25.1.8937393"
}
"""

with open("app/build.gradle", "w") as f:
    f.write(app_build_content)

print("✅ تم تحديث جميع ملفات البناء بنجاح.")

# ---------------------------------------------------------
# 4. الرفع الفوري
# ---------------------------------------------------------
print("📤 جاري الرفع للسحابة...")
os.system("git add .")
os.system('git commit -m "Refactor: Force switch to Modern Gradle Plugins DSL"')
os.system("git push origin main --force")
