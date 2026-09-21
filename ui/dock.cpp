#include "dock.h"


std::vector<std::string> windowTitles;

void getAllWindowVisibleTitle() {
    windowTitles.clear();
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        char title[256];
        GetWindowTextA(hwnd, title, sizeof(title));
        WINDOWPLACEMENT placement;
        if (IsWindowVisible(hwnd) && GetWindowPlacement(hwnd, &placement) && placement.showCmd != SW_HIDE) {
            windowTitles.push_back(std::string(title));
        }
        return TRUE;
    }, 0);

    for (const auto& title : windowTitles) {
        std::cout << "Visible Window Title: " << title << std::endl;
    }
}