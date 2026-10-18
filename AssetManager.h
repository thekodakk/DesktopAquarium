#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <memory>

class AssetManager {
public:
    static void Initialize();
    static void Shutdown();

    static Gdiplus::Bitmap* GetBackground();
    static Gdiplus::Bitmap* GetGoldfish();
    static Gdiplus::Bitmap* GetClownfish();
    static Gdiplus::Bitmap* GetBlueTang();
    static Gdiplus::Bitmap* GetAxolotl();
    static Gdiplus::Bitmap* GetPlant();
    static Gdiplus::Bitmap* GetBubble();

private:
    static std::unique_ptr<Gdiplus::Bitmap> s_background;
    static std::unique_ptr<Gdiplus::Bitmap> s_goldfish;
    static std::unique_ptr<Gdiplus::Bitmap> s_clownfish;
    static std::unique_ptr<Gdiplus::Bitmap> s_blueTang;
    static std::unique_ptr<Gdiplus::Bitmap> s_axolotl;
    static std::unique_ptr<Gdiplus::Bitmap> s_plant;
    static std::unique_ptr<Gdiplus::Bitmap> s_bubble;

    static Gdiplus::Bitmap* LoadBitmapFromFile(const std::wstring& filename);
};

#endif // ASSETMANAGER_H
