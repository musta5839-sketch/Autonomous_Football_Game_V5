import os
import time

# نولد وقتاً حالياً لنجبر النظام على رؤية تغيير جديد
current_time = str(time.time())
print(f"🔥 بدء عملية التحديث الشامل (Timestamp: {current_time})...")

# ---------------------------------------------------------
# 1. تحديث ملف السحابة (Fix Artifact v3 Error)
# ---------------------------------------------------------
workflow_content = f"""name: Ultimate Build V4
on: [push, workflow_dispatch]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup JDK 17
        uses: actions/setup-java@v4
        with:
          java-version: '17'
          distribution: 'temurin'

      - name: Setup Gradle 8.2
        uses: gradle/actions/setup-gradle@v3
        with:
          gradle-version: '8.2'
          
      - name: Setup Android SDK
        uses: android-actions/setup-android@v3

      - name: Build APK
        run: gradle assembleDebug --no-daemon --stacktrace
        
      # هنا الحل: نستخدم الإصدار v4 حصرياً
      - name: Upload APK
        if: success()
        uses: actions/upload-artifact@v4
        with:
          name: FINAL-GAME-APK-{int(time.time())}
          path: app/build/outputs/apk/debug/app-debug.apk
"""
# نكتب الملف ونتأكد من إنشاء المجلد
os.makedirs(".github/workflows", exist_ok=True)
with open(".github/workflows/android.yml", "w") as f:
    f.write(workflow_content)

# ---------------------------------------------------------
# 2. ضبط ملفات Gradle على الوضع الكلاسيكي (للأمان)
# ---------------------------------------------------------
# settings.gradle
with open("settings.gradle", "w") as f:
    f.write(f"// Updated: {current_time}\nrootProject.name = 'Autonomous_Football_Game_V5'\ninclude ':app'")

# build.gradle (Root)
root_build = f"""// Force Update: {current_time}
buildscript {{
    repositories {{
        google()
        mavenCentral()
    }}
    dependencies {{
        classpath 'com.android.tools.build:gradle:8.1.0'
    }}
}}
allprojects {{
    repositories {{
        google()
        mavenCentral()
    }}
}}
task clean(type: Delete) {{
    delete rootProject.buildDir
}}
"""
with open("build.gradle", "w") as f:
    f.write(root_build)

# app/build.gradle
app_build = f"""// Force Update: {current_time}
apply plugin: 'com.android.application'

android {{
    namespace 'com.ai.autonomous.game'
    compileSdk 34

    defaultConfig {{
        applicationId "com.ai.autonomous.game"
        minSdk 24
        targetSdk 34
        versionCode 1
        versionName "1.0"

        externalNativeBuild {{
            cmake {{
                cppFlags "-std=c++20"
            }}
        }}
    }}

    externalNativeBuild {{
        cmake {{
            path "src/main/cpp/CMakeLists.txt"
        }}
    }}
    
    ndkVersion "25.1.8937393"
}}
"""
with open("app/build.gradle", "w") as f:
    f.write(app_build)

print("✅ تم تحديث جميع الملفات وإضافة بصمة وقت جديدة.")

# ---------------------------------------------------------
# 3. الرفع الإجباري
# ---------------------------------------------------------
print("📤 جاري الرفع للسحابة...")
os.system("git add .")
os.system('git commit -m "Ultimate Fix: Upgrade Artifacts to v4 and Reset Gradle"')
os.system("git push origin main --force")
