import os

# كود البناء المصحح 100%
workflow_content = """name: Final Victory Build
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
          
      - name: Build APK
        run: gradle assembleDebug
        
      - name: Upload APK
        uses: actions/upload-artifact@v4
        with:
          name: Final-Game-APK
          path: app/build/outputs/apk/debug/app-debug.apk
"""

# كتابة الملف
with open(".github/workflows/android.yml", "w") as f:
    f.write(workflow_content)

print("✅ تم تصحيح رابط Gradle بنجاح!")
