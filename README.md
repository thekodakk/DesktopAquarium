# DesktopAquarium

Windows masaüstünde animasyonlu balıklar ve akvaryum görselleri gösteren C++17 uygulaması. Win32, GDI ve GDI+ kullanır; görseller `assets/` klasöründedir.

## Derleme

Windows ve PATH üzerinde MinGW-w64 `g++` derleyicisi gerekir. Proje kökünde PowerShell ile:

```powershell
.\build.ps1
.\DesktopAquarium.exe
```

Veya doğrudan:

```powershell
g++ -O2 -std=c++17 main.cpp Aquarium.cpp Fish.cpp Renderer.cpp ContextMenu.cpp AssetManager.cpp -o DesktopAquarium.exe -lgdi32 -lgdiplus -luser32 -lshell32 -mwindows
```

Uygulamayı `assets/` klasörü yanında olacak şekilde proje kökünden çalıştırın. Derlenmiş uygulama ve yerel `tools/` derleyici paketi depoya dahil edilmemiştir. Mevcut CMake ve MSVC derleme tanımlarının kaynak listeleri GCC komutundan farklıdır; yukarıdaki GCC yolu tercih edilir.
