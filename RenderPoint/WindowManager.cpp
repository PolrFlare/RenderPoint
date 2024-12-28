#include "WindowManager.h"
#include "Direct2DGraphics.h"

WindowManager::WindowManager(HINSTANCE hInstance) : hInstance(hInstance), hwnd(nullptr), hwnd2(nullptr) {}

WindowManager::~WindowManager() {}

void WindowManager::CreateMainWindow(int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Direct2DWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"Direct2D Always On Top", WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance, nullptr);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    CenterWindow(hwnd);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

void WindowManager::CreateSecondaryWindow()
{
    const wchar_t CLASS_NAME[] = L"SecondaryWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProcSecondary;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    hwnd2 = CreateWindowEx(
        0, CLASS_NAME, L"Secondary Window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd2, SW_SHOWNORMAL);
    UpdateWindow(hwnd2);
}

void WindowManager::CenterWindow(HWND hwnd)
{
    RECT rcScreen, rcWindow;
    GetWindowRect(GetDesktopWindow(), &rcScreen);
    GetWindowRect(hwnd, &rcWindow);

    int screenWidth = rcScreen.right - rcScreen.left;
    int screenHeight = rcScreen.bottom - rcScreen.top;

    int windowWidth = rcWindow.right - rcWindow.left;
    int windowHeight = rcWindow.bottom - rcWindow.top;

    int xPos = (screenWidth - windowWidth) / 2;
    int yPos = (screenHeight - windowHeight) / 2;

    SetWindowPos(hwnd, HWND_TOPMOST, xPos, yPos, 0, 0, SWP_NOSIZE);
}

LRESULT CALLBACK WindowManager::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        if (pRenderTarget != nullptr)
        {
            pRenderTarget->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
        }
        return 0;

    case WM_PAINT:
        Render();
        ValidateRect(hwnd, nullptr);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK WindowManager::WndProcSecondary(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
