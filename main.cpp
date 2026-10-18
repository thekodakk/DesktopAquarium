#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include "Aquarium.h"
#include "Renderer.h"
#include "ContextMenu.h"
#include "AssetManager.h"

// Link GDI+ and Windows libraries automatically
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")

static const wchar_t CLASS_NAME[] = L"DesktopAquariumWindowClass";
static const wchar_t WINDOW_TITLE[] = L"Masaüstü Akvaryum (Desktop Aquarium)";

// Global state pointers
static std::unique_ptr<Aquarium> g_aquarium;
static ULONG_PTR g_gdiplusToken = 0;

// Mouse Drag State
static bool g_isDragging = false;
static POINT g_dragStartMousePos;
static POINT g_dragStartWindowPos;

// High-resolution clock timing
static LARGE_INTEGER g_clockFrequency;
static LARGE_INTEGER g_lastClockTime;

// Layered Window Rendering Resources
static HDC g_memDC = NULL;
static HBITMAP g_memBitmap = NULL;
static HBITMAP g_oldBitmap = NULL;
static void* g_bitmapBits = NULL;
static int g_windowWidth = 520;
static int g_windowHeight = 360;

void InitMemoryDC(HWND hwnd, int width, int height) {
    HDC hdcScreen = GetDC(hwnd);
    g_memDC = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;     // 32-bit ARGB per-pixel alpha
    bmi.bmiHeader.biCompression = BI_RGB;

    g_memBitmap = CreateDIBSection(g_memDC, &bmi, DIB_RGB_COLORS, &g_bitmapBits, NULL, 0);
    g_oldBitmap = (HBITMAP)SelectObject(g_memDC, g_memBitmap);

    ReleaseDC(hwnd, hdcScreen);
}

void CleanupMemoryDC() {
    if (g_memDC) {
        SelectObject(g_memDC, g_oldBitmap);
        DeleteObject(g_memBitmap);
        DeleteDC(g_memDC);
        g_memDC = NULL;
    }
}

void RenderAndDrawWindow(HWND hwnd) {
    if (!g_memDC || !g_aquarium) return;

    // Clear background to 100% transparent alpha (0x00000000)
    memset(g_bitmapBits, 0, g_windowWidth * g_windowHeight * 4);

    // GDI+ Graphics wrapper on 32-bit memory DC
    {
        Gdiplus::Graphics graphics(g_memDC);
        Renderer::DrawAquarium(graphics, *g_aquarium);
    }

    // Update Layered Window with per-pixel alpha blending
    POINT ptSrc = {0, 0};
    SIZE sizeWnd = {g_windowWidth, g_windowHeight};
    BLENDFUNCTION blend = {0};
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA; // Per-pixel alpha channel

    RECT rcWnd;
    GetWindowRect(hwnd, &rcWnd);
    POINT ptDst = {rcWnd.left, rcWnd.top};

    HDC hdcScreen = GetDC(hwnd);
    UpdateLayeredWindow(hwnd, hdcScreen, &ptDst, &sizeWnd, g_memDC, &ptSrc, 0, &blend, ULW_ALPHA);
    ReleaseDC(hwnd, hdcScreen);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            InitMemoryDC(hwnd, g_windowWidth, g_windowHeight);

            // Center window on screen initially
            int screenW = GetSystemMetrics(SM_CXSCREEN);
            int screenH = GetSystemMetrics(SM_CYSCREEN);
            int startX = (screenW - g_windowWidth) / 2;
            int startY = (screenH - g_windowHeight) / 2;

            SetWindowPos(hwnd, HWND_TOPMOST, startX, startY, g_windowWidth, g_windowHeight, SWP_NOACTIVATE | SWP_SHOWWINDOW);
            SetTimer(hwnd, 1, 16, NULL); // ~60 FPS update timer tick
            return 0;
        }

        case WM_TIMER: {
            // Calculate delta time
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            float dt = (float)(currentTime.QuadPart - g_lastClockTime.QuadPart) / (float)g_clockFrequency.QuadPart;
            g_lastClockTime = currentTime;

            if (dt > 0.1f) dt = 0.1f; // Cap max delta time

            RECT rcWnd;
            GetWindowRect(hwnd, &rcWnd);

            // Update Aquarium physics & state
            g_aquarium->Update(dt, rcWnd.left, rcWnd.top, g_isDragging);

            // Render to desktop
            RenderAndDrawWindow(hwnd);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int clickX = LOWORD(lParam);
            int clickY = HIWORD(lParam);

            // Check if interactive Feed button was clicked
            if (g_aquarium->IsFeedButtonClicked(clickX, clickY)) {
                g_aquarium->FeedFish();
                return 0;
            }

            // Start Dragging Aquarium around Desktop
            g_isDragging = true;
            SetCapture(hwnd);

            GetCursorPos(&g_dragStartMousePos);
            RECT rcWnd;
            GetWindowRect(hwnd, &rcWnd);
            g_dragStartWindowPos.x = rcWnd.left;
            g_dragStartWindowPos.y = rcWnd.top;
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (g_isDragging) {
                POINT currentMousePos;
                GetCursorPos(&currentMousePos);

                int dx = currentMousePos.x - g_dragStartMousePos.x;
                int dy = currentMousePos.y - g_dragStartMousePos.y;

                int newWindowX = g_dragStartWindowPos.x + dx;
                int newWindowY = g_dragStartWindowPos.y + dy;

                SetWindowPos(hwnd, HWND_TOPMOST, newWindowX, newWindowY, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            if (g_isDragging) {
                g_isDragging = false;
                ReleaseCapture();
            }
            return 0;
        }

        case WM_RBUTTONUP: {
            // Open Right-Click Context Menu for selecting fish & options
            POINT pt;
            GetCursorPos(&pt);
            ContextMenu::Show(hwnd, pt.x, pt.y, *g_aquarium);
            return 0;
        }

        case WM_COMMAND: {
            WORD commandId = LOWORD(wParam);
            ContextMenu::HandleCommand(commandId, *g_aquarium, hwnd);
            return 0;
        }

        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            CleanupMemoryDC();
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Initialize GDI+ & Asset Manager
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    if (Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::Ok) {
        MessageBoxW(NULL, L"GDI+ Initialization Failed!", L"Error", MB_ICONERROR);
        return 1;
    }
    AssetManager::Initialize();

    // 2. Query High-Resolution Clock
    QueryPerformanceFrequency(&g_clockFrequency);
    QueryPerformanceCounter(&g_lastClockTime);

    // 3. Create Aquarium Engine
    g_aquarium = std::make_unique<Aquarium>(g_windowWidth, g_windowHeight);

    // 4. Register Layered Window Class
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONERROR);
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        return 1;
    }

    // 5. Create Frameless Layered Topmost Window
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        WINDOW_TITLE,
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, g_windowWidth, g_windowHeight,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONERROR);
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 6. Message Loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 7. Cleanup
    g_aquarium.reset();
    AssetManager::Shutdown();
    Gdiplus::GdiplusShutdown(g_gdiplusToken);

    return (int)msg.wParam;
}
