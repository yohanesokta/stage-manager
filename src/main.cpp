#include "main.h"

HWND getWindowFocus()
{
    HWND hwnd = GetForegroundWindow();
    return hwnd;
}

void SetWindowFocusAsMinimize(HWND hwnd)
{
    bool isWindowZoomed = IsZoomed(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    if (isWindowZoomed)
    {
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
}

std::vector<HWND> hwnds;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    if (IsWindowVisible(hwnd) && strlen(title) > 0 && title != std::string("Program Manager"))
    {
        hwnds.push_back(hwnd);
    }
    return TRUE;
}

void focusAndMinimizeAllWindow(HWND hwndNow)
{
    hwnds.clear();
    EnumWindows(EnumWindowsProc, 0);
    for (HWND hwnd : hwnds)
    {
        if (hwnd != hwndNow)
        {
            ShowWindow(hwnd, SW_MINIMIZE);
        }
    }
    Sleep(100);
    char title[256];
    GetWindowTextA(hwndNow, title, sizeof(title));
    std::cout << "Window NOW Title: " << title << std::endl;
    SetWindowFocusAsMinimize(hwndNow);
}

void mainLoop()
{
    std::string windowsFeaturesTittle[FEATURES_WINDOWS_PROGRAM_SIZE];
    windowsFeaturesTittle[0] = "Program Manager";
    windowsFeaturesTittle[1] = "Task Switching";
    getAllWindowVisibleTitle();

    HWND hwndNow = getWindowFocus();
    focusAndMinimizeAllWindow(hwndNow);
    while (true)
    {
        HWND SwitchWindow = GetForegroundWindow();
        char title[256];
        GetWindowTextA(SwitchWindow, title, sizeof(title));
        bool isWindowFeatures = false;
        for (int i = 0; i < FEATURES_WINDOWS_PROGRAM_SIZE; i++)
        {
            if (windowsFeaturesTittle[i] == title)
            {
                isWindowFeatures = true;
            }
        }
        if (GetForegroundWindow() != hwndNow && !isWindowFeatures)
        {
            Sleep(100);
            hwndNow = getWindowFocus();
            focusAndMinimizeAllWindow(GetForegroundWindow());
        }
        Sleep(16);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    window.setWindowTitle("Stage Manager");
    window.resize(400, 300);
    window.show();
    QThread* workerThread = QThread::create([]() {
        mainLoop();
    });
     app.exec();
}