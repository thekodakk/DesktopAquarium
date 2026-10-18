#include "ContextMenu.h"

void ContextMenu::Show(HWND hwnd, int screenX, int screenY, Aquarium& aquarium) {
    HMENU hMenu = CreatePopupMenu();
    HMENU hSubMenuFish = CreatePopupMenu();

    // 1. Add 10 Fish Species checkboxes to submenu
    for (int i = 0; i < FISH_COUNT; ++i) {
        FishType type = static_cast<FishType>(i);
        const auto& info = GetFishInfo(type);

        UINT flags = MF_STRING;
        if (aquarium.IsFishTypeActive(type)) {
            flags |= MF_CHECKED;
        }

        std::wstring menuItemText = info.name + L" (" + info.description + L")";
        AppendMenuW(hSubMenuFish, flags, ID_FISH_BASE + i, menuItemText.c_str());
    }

    // Main popup menu structure
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenuFish, L" Balık Türlerini Seç (10 Çeşit)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hMenu, MF_STRING, ID_SELECT_ALL_FISH, L" Tüm Balıkları Göster");
    AppendMenuW(hMenu, MF_STRING, ID_DESELECT_ALL_FISH, L" Tüm Balıkları Gizle");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hMenu, MF_STRING, ID_FEED_FISH, L" Balıkları Besle (Yem At)");
    AppendMenuW(hMenu, MF_STRING, ID_REFILL_WATER, L" Suyu Yeniden Doldur (%100 Su)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hMenu, MF_STRING, ID_EXIT_APP, L" Çıkış (Aquarium Exit)");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN, screenX, screenY, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

void ContextMenu::HandleCommand(WORD commandId, Aquarium& aquarium, HWND hwnd) {
    if (commandId >= ID_FISH_BASE && commandId < ID_FISH_BASE + FISH_COUNT) {
        FishType type = static_cast<FishType>(commandId - ID_FISH_BASE);
        bool currentState = aquarium.IsFishTypeActive(type);
        aquarium.SetFishTypeActive(type, !currentState);
    } 
    else if (commandId == ID_FEED_FISH) {
        aquarium.FeedFish();
    }
    else if (commandId == ID_REFILL_WATER) {
        aquarium.RefillWater();
    }
    else if (commandId == ID_SELECT_ALL_FISH) {
        aquarium.ToggleAllFish(true);
    }
    else if (commandId == ID_DESELECT_ALL_FISH) {
        aquarium.ToggleAllFish(false);
    }
    else if (commandId == ID_EXIT_APP) {
        PostQuitMessage(0);
    }
}
