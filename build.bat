@echo off
chcp 65001 >NUL
echo ============================================================
echo   minoreOptimizer v2.0 - Standalone EXE Build
echo ============================================================
echo.

pip install PyQt6 pyinstaller --quiet

echo [1/2] Building standalone EXE (no Python required)...

pyinstaller --onefile --windowed --name "minoreOptimizer" --icon "icon.ico" --version-file "version_info.txt" --uac-admin --add-data "icon.png;." --add-data "github.png;." --add-data "msi.png;." --add-data "deepsek.png;." --add-data "restore.png;." --collect-all PyQt6 --hidden-import "PyQt6.QtCore" --hidden-import "PyQt6.QtGui" --hidden-import "PyQt6.QtWidgets" --hidden-import "PyQt6.QtNetwork" --hidden-import "PyQt6.sip" --hidden-import "winreg" --hidden-import "ctypes" --hidden-import "ctypes.wintypes" --hidden-import "json" --hidden-import "subprocess" --hidden-import "threading" --hidden-import "webbrowser" --hidden-import "gc" --hidden-import "re" --hidden-import "time" --hidden-import "platform" optimizer.py

echo.
echo [2/2] Done! EXE: dist\minoreOptimizer.exe
echo.
pause
