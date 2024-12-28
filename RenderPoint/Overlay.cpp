#include <windows.h>
#include <d2d1.h>
#include <dwmapi.h>
#include <math.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwmapi.lib")

ID2D1Factory* pD2DFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;
HWND hwnd;
HWND hwnd2;

float hue = 0.0f;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcSecondary(HWND, UINT, WPARAM, LPARAM);
void InitializeDirect2D();
void CreateGraphicsResources();
void DiscardGraphicsResources();
void Render();
void UpdateRainbowColor();
void DrawCrosshair(float centerX, float centerY, float size);
void CenterWindow(HWND hwnd);
void configWindow(HINSTANCE hInstance);

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        if (pRenderTarget != nullptr)
        {
            //resize
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

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK WndProcSecondary(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // White background

    switch (uMsg)
    {
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT); // Make text background transparent
        return (LRESULT)hBrush;            // Use a white brush for background
    }
    case WM_DESTROY:
        DeleteObject(hBrush); // Clean up resources
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

//init d2d
void InitializeDirect2D()
{
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
}

void CreateGraphicsResources()
{
    if (pRenderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        //create crosshair
        pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
            &pRenderTarget
        );

        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pBrush);
    }
}

void DiscardGraphicsResources()
{
    if (pRenderTarget)
    {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    if (pBrush)
    {
        pBrush->Release();
        pBrush = nullptr;
    }
}

//chroma effect
void UpdateRainbowColor()
{
    hue += 0.005f;
    if (hue > 1.0f)
    {
        hue = 0.0f;
    }

    float r = 0.0f, g = 0.0f, b = 0.0f;
    float p, q, t, frac;

    frac = hue * 6.0f;
    int i = static_cast<int>(frac);
    t = frac - i;
    p = 0.0f;
    q = 1.0f - t;
    switch (i)
    {
    case 0: r = 1.0f; g = t; b = p; break;
    case 1: r = q; g = 1.0f; b = p; break;
    case 2: r = p; g = 1.0f; b = t; break;
    case 3: r = p; g = q; b = 1.0f; break;
    case 4: r = t; g = p; b = 1.0f; break;
    case 5: r = 1.0f; g = p; b = q; break;
    }

    pBrush->SetColor(D2D1::ColorF(r, g, b));
}

// Draw a crosshair in the center of the window
void DrawCrosshair(float centerX, float centerY, float size)
{
    // Horizontal line of the crosshair
    pRenderTarget->FillRectangle(D2D1::RectF(centerX - size / 2, centerY - 1, centerX + size / 2, centerY + 1), pBrush);

    // Vertical line of the crosshair
    pRenderTarget->FillRectangle(D2D1::RectF(centerX - 1, centerY - size / 2, centerX + 1, centerY + size / 2), pBrush);
}

//render crosshair
void Render()
{
    if (pRenderTarget == nullptr)
    {
        CreateGraphicsResources();
    }

    // Update the rainbow color for the animation
    UpdateRainbowColor();

    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));  // Transparent background

    // Draw the crosshair at the center of the window
    RECT rc;
    GetClientRect(hwnd, &rc);
    float centerX = (rc.right - rc.left) / 2.0f;
    float centerY = (rc.bottom - rc.top) / 2.0f;
    float size = 15.0f;  // Size of the crosshair

    DrawCrosshair(centerX, centerY, size);

    pRenderTarget->EndDraw();
}

void CenterWindow(HWND hwnd)
{
    RECT rcScreen;
    GetWindowRect(GetDesktopWindow(), &rcScreen);
    int screenWidth = rcScreen.right - rcScreen.left;
    int screenHeight = rcScreen.bottom - rcScreen.top;

    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);
    int windowWidth = rcWindow.right - rcWindow.left;
    int windowHeight = rcWindow.bottom - rcWindow.top;

    int xPos = (screenWidth - windowWidth) / 2;
    int yPos = (screenHeight - windowHeight) / 2;

    SetWindowPos(hwnd, HWND_TOPMOST, xPos, yPos, 0, 0, SWP_NOSIZE);
}

//config window
void configWindow(HINSTANCE hInstance)
{
    const wchar_t CLASS_NAME[] = L"Config";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProcSecondary;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    //soon we want to be able to hid the window instead of closing it, so wed only use ws iconics
    hwnd2 = CreateWindowEx(
        0, CLASS_NAME, L"Config", WS_ICONIC | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, nullptr, nullptr, hInstance, nullptr);

    HWND colorText = CreateWindowEx(
        0, L"STATIC", L"Color", WS_CHILD | WS_VISIBLE | SS_CENTER,
        50, 20, 100, 20, hwnd2, nullptr, hInstance, nullptr
    );

    HFONT hFont = CreateFont(
        20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Helvetica"
    );
    SendMessage(colorText, WM_SETFONT, (WPARAM)hFont, TRUE);
    SetLayeredWindowAttributes(colorText, RGB(255, 0, 0), 0, LWA_COLORKEY);

    HWND hwndTextbox = CreateWindowEx(
        0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        160, 20, 150, 21, hwnd2, (HMENU)1, hInstance, nullptr);

    ShowWindow(hwnd2, SW_SHOWNORMAL);
    UpdateWindow(hwnd2);
}

//main
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Direct2DWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    //borderless and transparent
    hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"Direct2D Always On Top", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    CenterWindow(hwnd);
    InitializeDirect2D();
    configWindow(hInstance);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        InvalidateRect(hwnd, nullptr, TRUE);
    }

    DiscardGraphicsResources();
    pD2DFactory->Release();

    return 0;
}
