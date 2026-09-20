#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <vector>


HWND getWindowFocus() {
    HWND hwnd = GetForegroundWindow();
    return hwnd;
}

void SetWindowFocusAsMinimize(HWND hwnd) {
    // set the focus to the specified window and minimize it
    ShowWindow(hwnd, SW_RESTORE);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
}

std::vector<HWND> hwnds;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    if (IsWindowVisible(hwnd) && strlen(title) > 0 && title != std::string("Program Manager")) {
        hwnds.push_back(hwnd);
    }
    return TRUE; 
}


void focusAndMinimizeAllWindow(HWND hwndNow) {
    hwnds.clear();    
    EnumWindows(EnumWindowsProc, 0);
    for (HWND hwnd : hwnds) {
        if (hwnd != hwndNow) {
            ShowWindow(hwnd, SW_MINIMIZE);
        }
    }
    Sleep(100);
    char title[256];
    GetWindowTextA(hwndNow, title, sizeof(title));
    std::cout << "Window NOW Title: " << title << std::endl;
    SetWindowFocusAsMinimize(hwndNow);
}

int main(int argc,char* argv[]) {
    // HWND hwnd = FindWindowA(NULL, "File Explorer");
    // if(IsWindow(hwnd)) {
    //     std::cout << "File Explorer window found!" << std::endl;
    //     SetForegroundWindow(hwnd);
    //     Sleep(3000);
    //     ShowWindow(hwnd, SW_MINIMIZE);

    // } else {
    //     std::cout << "File Explorer window not found." << std::endl;
    // }

    HWND hwndNow = getWindowFocus();
    focusAndMinimizeAllWindow(hwndNow);
    while (true) {
        HWND SwitchWindow = GetForegroundWindow();
        char title[256];
        GetWindowTextA(SwitchWindow, title, sizeof(title));
        if (GetForegroundWindow() != hwndNow && std::string(title) != std::string("Program Manager")) {
            Sleep(100);
            hwndNow = getWindowFocus();
            focusAndMinimizeAllWindow(GetForegroundWindow());
        }
        Sleep(16);
    }
    return 0;
}