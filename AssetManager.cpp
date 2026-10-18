#include "AssetManager.h"

std::unique_ptr<Gdiplus::Bitmap> AssetManager::s_background = nullptr;
std::unique_ptr<Gdiplus::Bitmap> AssetManager::s_goldfish = nullptr;
std::unique_ptr<Gdiplus::Bitmap> AssetManager::s_clownfish = nullptr;
std::unique_ptr<Gdiplus::Bitmap> AssetManager::s_blueTang = nullptr;
std::unique_ptr<Gdiplus::Bitmap> AssetManager::s_axolotl = nullptr;
std::unique_ptr<Gdiplus::Bitmap> AssetManager::s_plant = nullptr;
std::unique_ptr<Gdiplus::Bitmap> AssetManager::s_bubble = nullptr;

Gdiplus::Bitmap* AssetManager::LoadBitmapFromFile(const std::wstring& filename) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring path(exePath);
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        path = path.substr(0, pos);
    }
    path += L"\\assets\\" + filename;

    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(path.c_str());
    if (bmp && bmp->GetLastStatus() == Gdiplus::Ok) {
        return bmp;
    }
    delete bmp;
    return nullptr;
}

void AssetManager::Initialize() {
    s_background.reset(LoadBitmapFromFile(L"background.png"));
    s_goldfish.reset(LoadBitmapFromFile(L"goldfish.png"));
    s_clownfish.reset(LoadBitmapFromFile(L"clownfish.png"));
    s_blueTang.reset(LoadBitmapFromFile(L"blue_tang.png"));
    s_axolotl.reset(LoadBitmapFromFile(L"axolotl.png"));
    s_plant.reset(LoadBitmapFromFile(L"plant.png"));
    s_bubble.reset(LoadBitmapFromFile(L"bubble.png"));
}

void AssetManager::Shutdown() {
    s_background.reset();
    s_goldfish.reset();
    s_clownfish.reset();
    s_blueTang.reset();
    s_axolotl.reset();
    s_plant.reset();
    s_bubble.reset();
}

Gdiplus::Bitmap* AssetManager::GetBackground() { return s_background.get(); }
Gdiplus::Bitmap* AssetManager::GetGoldfish() { return s_goldfish.get(); }
Gdiplus::Bitmap* AssetManager::GetClownfish() { return s_clownfish.get(); }
Gdiplus::Bitmap* AssetManager::GetBlueTang() { return s_blueTang.get(); }
Gdiplus::Bitmap* AssetManager::GetAxolotl() { return s_axolotl.get(); }
Gdiplus::Bitmap* AssetManager::GetPlant() { return s_plant.get(); }
Gdiplus::Bitmap* AssetManager::GetBubble() { return s_bubble.get(); }
