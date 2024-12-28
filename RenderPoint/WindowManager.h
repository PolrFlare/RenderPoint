#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <windows.h>

class WindowManager
{
public:
    WindowManager(HINSTANCE hInstance);
    ~WindowManager();
    void CreateMainWindow(int nCmdShow);
    void CreateSecondaryWindow();
    HWND GetMainWindow() const { return hwnd; }

private:
    HWND hwnd;
    HWND hwnd2;
    HINSTANCE hInstance;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK WndProcSecondary(HWND, UINT, WPARAM, LPARAM); // Secondary window WndProc
    void CenterWindow(HWND hwnd);
};

#endif // WINDOWMANAGER_H
