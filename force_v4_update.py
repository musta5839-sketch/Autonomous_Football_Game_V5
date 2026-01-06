import os

# محتوى الملف الجديد المحدث (V4)
yaml_content = """name: Final Fix Actions V4
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
        
      - name: Upload APK
        if: success()
        uses: actions/upload-artifact@v4
        with:
          name: WORKING-GAME-APK
          path: app/build/outputs/apk/debug/app-debug.apk
"""

# 1. حذف الملف القديم للتأكد من التغيير
file_path = ".github/workflows/android.yml"
if os.path.exists(file_path):
    os.remove(file_path)
    print("🗑️  تم حذف الملف القديم.")

# 2. إنشاء المجلد إذا لم يكن موجوداً
os.makedirs(os.path.dirname(file_path), exist_ok=True)

# 3. كتابة الملف الجديد
with open(file_path, "w", encoding="utf-8") as f:
    f.write(yaml_content)
print("✅ تم كتابة ملف V4 الجديد بنجاح.")

# 4. تنفيذ أوامر Git تلقائياً
print("📤 جاري الرفع...")
os.system("git add .")
os.system('git commit -m "Force Update: Rewrite workflow with Actions V4"')
os.system("git push origin main --force")
