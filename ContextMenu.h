#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include <windows.h>
#include "Aquarium.h"

// Context Menu Command Identifiers
#define ID_FISH_BASE 100
#define ID_FEED_FISH 200
#define ID_REFILL_WATER 201
#define ID_SELECT_ALL_FISH 202
#define ID_DESELECT_ALL_FISH 203
#define ID_EXIT_APP 300

class ContextMenu {
public:
    static void Show(HWND hwnd, int screenX, int screenY, Aquarium& aquarium);
    static void HandleCommand(WORD commandId, Aquarium& aquarium, HWND hwnd);
};

#endif // CONTEXTMENU_H
