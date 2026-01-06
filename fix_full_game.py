import os

# 1. ملف إعدادات اللعبة (كما هو)
gradle_build_content = """
plugins {
    id 'com.android.application' version '8.1.0'
}

android {
    namespace 'com.autonomous.football'
    compileSdk 33

    defaultConfig {
        applicationId "com.autonomous.football"
        minSdk 24
        targetSdk 33
        versionCode 1
        versionName "1.0"
    }
}
"""

# 2. ملف البناء السحابي (تم التحديث إلى v4 لتجاوز الخطأ)
workflow_content = """name: Final V4 Build
on: [push]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup Java 17
        uses: actions/setup-java@v4
        with:
          distribution: 'temurin'
          java-version: '17'
          
      - name: Setup Gradle
        uses: gradle/gradle-build-action@v2
        with:
          gradle-version: '8.2'
          
      - name: Create wrapper
        run: gradle wrapper
        
      - name: Build APK
        run: ./gradlew assembleDebug
        
      - name: Upload APK
        uses: actions/upload-artifact@v4
        with:
          name: Final-Game-APK
          path: app/build/outputs/apk/debug/app-debug.apk
"""

# كتابة الملفات
with open("app/build.gradle", "w") as f:
    f.write(gradle_build_content)

os.makedirs(".github/workflows", exist_ok=True)
with open(".github/workflows/android.yml", "w") as f:
    f.write(workflow_content)

print("✅ تم التحديث إلى الإصدار v4 بنجاح!")
