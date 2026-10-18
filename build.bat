@echo off
chcp 65001 > nul
echo ===================================================
echo     Desktop Aquarium Build Script
echo ===================================================
echo.

rem Check GCC / MinGW g++
where g++ >nul 2>nul
if %errorlevel% equ 0 goto USE_GCC

rem Check MSVC cl.exe
where cl >nul 2>nul
if %errorlevel% equ 0 goto USE_MSVC

echo [HATA] g++ veya cl derleyicisi bulunamadi.
echo Lutfen MinGW (g++) veya Visual Studio C++ yukleyin.
pause
exit /b 1

:USE_GCC
echo [INFO] MinGW GCC (g++) ile derleniyor...
g++ -O2 -std=c++17 main.cpp Aquarium.cpp Fish.cpp Renderer.cpp ContextMenu.cpp AssetManager.cpp -o DesktopAquarium.exe -lgdi32 -lgdiplus -luser32 -lshell32 -mwindows
if %errorlevel% equ 0 (
    echo.
    echo [BASARILI] DesktopAquarium.exe olusturuldu!
    echo Uygulamayi calistirmak icin DesktopAquarium.exe dosyasina tiklayin.
    pause
    exit /b 0
) else (
    echo.
    echo [HATA] MinGW derlemesi basarisiz oldu.
    pause
    exit /b 1
)

:USE_MSVC
echo [INFO] MSVC (cl.exe) ile derleniyor...
cl /O2 /EHsc /std:c++17 main.cpp Aquarium.cpp Fish.cpp Renderer.cpp ContextMenu.cpp /Fe:DesktopAquarium.exe gdi32.lib gdiplus.lib user32.lib shell32.lib /subsystem:windows
if %errorlevel% equ 0 (
    echo.
    echo [BASARILI] DesktopAquarium.exe olusturuldu!
    echo Uygulamayi calistirmak icin DesktopAquarium.exe dosyasina tiklayin.
    pause
    exit /b 0
) else (
    echo.
    echo [HATA] MSVC derlemesi basarisiz oldu.
    pause
    exit /b 1
)
