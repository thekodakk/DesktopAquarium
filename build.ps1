# PowerShell Build Script for Desktop Aquarium
Set-Location -Path $PSScriptRoot

Write-Host "===================================================" -ForegroundColor Cyan
Write-Host "     Desktop Aquarium PowerShell Build Script" -ForegroundColor Cyan
Write-Host "===================================================" -ForegroundColor Cyan
Write-Host ""

if (Get-Command g++ -ErrorAction SilentlyContinue) {
    Write-Host "[INFO] MinGW GCC (g++) bulundu. Derleme başlatılıyor..." -ForegroundColor Yellow
    g++ -O2 -std=c++17 main.cpp Aquarium.cpp Fish.cpp Renderer.cpp ContextMenu.cpp AssetManager.cpp -o DesktopAquarium.exe -lgdi32 -lgdiplus -luser32 -lshell32 -mwindows
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n[BAŞARILI] DesktopAquarium.exe oluşturuldu!" -ForegroundColor Green
        Write-Host "Çalıştırmak için: .\DesktopAquarium.exe" -ForegroundColor White
        exit 0
    } else {
        Write-Host "`n[HATA] MinGW derlemesi başarısız oldu." -ForegroundColor Red
        exit 1
    }
} elseif (Get-Command cl -ErrorAction SilentlyContinue) {
    Write-Host "[INFO] MSVC (cl.exe) bulundu. Derleme başlatılıyor..." -ForegroundColor Yellow
    cl /O2 /EHsc /std:c++17 main.cpp Aquarium.cpp Fish.cpp Renderer.cpp ContextMenu.cpp /Fe:DesktopAquarium.exe gdi32.lib gdiplus.lib user32.lib shell32.lib /subsystem:windows
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n[BAŞARILI] DesktopAquarium.exe oluşturuldu!" -ForegroundColor Green
        Write-Host "Çalıştırmak için: .\DesktopAquarium.exe" -ForegroundColor White
        exit 0
    } else {
        Write-Host "`n[HATA] MSVC derlemesi başarısız oldu." -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "[HATA] Sisteminizde g++ veya cl derleyicisi bulunamadı." -ForegroundColor Red
    Write-Host "Lütfen MinGW (g++) veya Visual Studio C++ (cl.exe) yükleyin." -ForegroundColor Red
    exit 1
}
