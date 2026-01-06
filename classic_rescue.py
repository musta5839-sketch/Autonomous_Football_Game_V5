import os
import time

print("🛡️  العودة إلى النظام الكلاسيكي المضمون (Classic Repair)...")

# 1. تنظيف settings.gradle (إزالة التعقيدات الحديثة)
settings_content = """rootProject.name = "Autonomous_Football_Game_V5"
include ':app'
"""
with open("settings.gradle", "w") as f:
    f.write(settings_content)

# 2. ملف البناء الرئيسي (الطريقة الكلاسيكية: buildscript)
root_build_content = """// تم التحديث بتاريخ: """ + str(time.time()) + """
buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        // نستخدم النسخة 8.1.0 المستقرة
        classpath 'com.android.tools.build:gradle:8.1.0'
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
    }
}

task clean(type: Delete) {
    delete rootProject.buildDir
}
"""
with open("build.gradle", "w") as f:
    f.write(root_build_content)

# 3. ملف التطبيق (إضافة المكتبات الضرورية + الطريقة الكلاسيكية)
app_build_content = """apply plugin: 'com.android.application'

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
    
    ndkVersion "25.1.8937393"
}

dependencies {
    // إضافة مكتبات أساسية لمنع أخطاء التشغيل
    implementation 'androidx.appcompat:appcompat:1.6.1'
    implementation 'com.google.android.material:material:1.9.0'
}
"""
with open("app/build.gradle", "w") as f:
    f.write(app_build_content)

print("✅ تم استبدال الملفات بالنسخة الكلاسيكية.")

# 4. الرفع الإجباري
print("📤 جاري دفع التحديث الجديد...")
os.system("git add .")
os.system('git commit -m "Fix: Revert to Classic Gradle Layout to fix build error"')
os.system("git push origin main --force")
