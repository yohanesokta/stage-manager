#include "WindowManager.h"
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <psapi.h>
#include <QPixmap>
#include <QFileInfo>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")

static WindowManager* s_instance = nullptr;

WindowManager::WindowManager(QObject* parent) : QObject(parent) {
    s_instance = this;
}

WindowManager::~WindowManager() {
    stopHook();
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void CALLBACK WindowManager::winEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                           LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    Q_UNUSED(hWinEventHook);
    Q_UNUSED(dwEventThread);
    Q_UNUSED(dwmsEventTime);

    if (idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
        return;

    if (s_instance && hwnd) {
        if (event == EVENT_SYSTEM_FOREGROUND) {
            
            QMetaObject::invokeMethod(s_instance, [hwnd]() {
                emit s_instance->foregroundWindowChanged(hwnd);
            }, Qt::QueuedConnection);
        } else if (event == EVENT_OBJECT_DESTROY || event == EVENT_OBJECT_HIDE) {
            QMetaObject::invokeMethod(s_instance, [hwnd]() {
                emit s_instance->windowDestroyedOrHidden(hwnd);
            }, Qt::QueuedConnection);
        }
    }
}

void WindowManager::startHook() {
    if (m_hook) return;
    m_hook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
        NULL, WindowManager::winEventProc, 0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
    );
}

void WindowManager::stopHook() {
    if (m_hook) {
        UnhookWinEvent(m_hook);
        m_hook = nullptr;
    }
}

bool WindowManager::isUserWindow(HWND hwnd, HWND selfHwnd) {
    if (!hwnd || !IsWindow(hwnd)) return false;
    if (hwnd == selfHwnd) return false;

    
    if (!IsWindowVisible(hwnd)) return false;

    
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == GetCurrentProcessId()) return false;

    
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    
    if ((exStyle & WS_EX_TOOLWINDOW) && !(exStyle & WS_EX_APPWINDOW)) {
        return false;
    }

    
    if (style & WS_CHILD) return false;

    
    wchar_t title[256];
    int len = GetWindowTextW(hwnd, title, 256);
    if (len == 0) return false;

    
    wchar_t className[256];
    GetClassNameW(hwnd, className, 256);
    QString cls = QString::fromWCharArray(className);
    QString titleStr = QString::fromWCharArray(title);

    if (cls == "Progman" || cls == "WorkerW" || cls == "Shell_TrayWnd" ||
        cls == "Shell_SecondaryTrayWnd" || cls == "Windows.UI.Core.CoreWindow" ||
        cls == "MultitaskingViewFrame" || cls == "XamlExplorerHostIslandWindow" ||
        titleStr == "Program Manager" || titleStr == "Task Switching") {
        return false;
    }

    
    BOOL cloaked = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked)))) {
        if (cloaked) return false;
    }

    
    RECT rc;
    if (GetWindowRect(hwnd, &rc)) {
        if ((rc.right - rc.left) <= 0 || (rc.bottom - rc.top) <= 0) {
            return false;
        }
    }

    return true;
}

WindowState WindowManager::captureWindowState(HWND hwnd) {
    WindowState state;
    state.hwnd = hwnd;
    if (!hwnd || !IsWindow(hwnd)) return state;

    wchar_t titleBuf[256];
    GetWindowTextW(hwnd, titleBuf, 256);
    state.title = QString::fromWCharArray(titleBuf);

    wchar_t classBuf[256];
    GetClassNameW(hwnd, classBuf, 256);
    state.className = QString::fromWCharArray(classBuf);

    GetWindowThreadProcessId(hwnd, &state.processId);
    state.exePath = getProcessName(state.processId);
    state.icon = getWindowIcon(hwnd);

    state.placement.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwnd, &state.placement);
    state.isZoomed = IsZoomed(hwnd) != FALSE;
    state.isIconic = IsIconic(hwnd) != FALSE;
    state.isVisible = IsWindowVisible(hwnd) != FALSE;

    return state;
}

QIcon WindowManager::getWindowIcon(HWND hwnd) {
    HICON hIcon = nullptr;

    SendMessageTimeoutW(hwnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG, 100, (PDWORD_PTR)&hIcon);
    if (!hIcon) {
        SendMessageTimeoutW(hwnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 100, (PDWORD_PTR)&hIcon);
    }
    if (!hIcon) {
        hIcon = (HICON)GetClassLongPtrW(hwnd, GCLP_HICON);
    }
    if (!hIcon) {
        hIcon = (HICON)GetClassLongPtrW(hwnd, GCLP_HICONSM);
    }
    if (!hIcon) {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (hProcess) {
            wchar_t exePath[MAX_PATH];
            DWORD size = MAX_PATH;
            if (QueryFullProcessImageNameW(hProcess, 0, exePath, &size)) {
                SHFILEINFOW sfi = { 0 };
                if (SHGetFileInfoW(exePath, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON)) {
                    hIcon = sfi.hIcon;
                }
            }
            CloseHandle(hProcess);
        }
    }

    if (hIcon) {
        QPixmap pixmap = QPixmap::fromImage(QImage::fromHICON(hIcon));
        DestroyIcon(hIcon);
        if (!pixmap.isNull()) {
            return QIcon(pixmap);
        }
    }

    return QIcon();
}

QPixmap WindowManager::captureWindowSnapshot(HWND hwnd, QSize targetSize) {
    if (!hwnd || !IsWindow(hwnd)) return QPixmap();

    RECT rc;
    if (!GetWindowRect(hwnd, &rc)) return QPixmap();
    int srcW = rc.right - rc.left;
    int srcH = rc.bottom - rc.top;
    if (srcW <= 0 || srcH <= 0) return QPixmap();

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, srcW, srcH);
    HGDIOBJ hOld = SelectObject(hdcMem, hbm);

    BOOL success = PrintWindow(hwnd, hdcMem, 0x00000002);
    if (!success) {
        success = PrintWindow(hwnd, hdcMem, 0);
    }
    if (!success) {
        HDC hdcWin = GetWindowDC(hwnd);
        if (hdcWin) {
            BitBlt(hdcMem, 0, 0, srcW, srcH, hdcWin, 0, 0, SRCCOPY);
            ReleaseDC(hwnd, hdcWin);
            success = TRUE;
        }
    }

    QPixmap result;
    if (success) {
        QImage image = QImage::fromHBITMAP(hbm);
        if (!image.isNull()) {
            if (!targetSize.isEmpty()) {
                result = QPixmap::fromImage(image.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                result = QPixmap::fromImage(image);
            }
        }
    }

    SelectObject(hdcMem, hOld);
    DeleteObject(hbm);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return result;
}

QString WindowManager::getProcessName(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        wchar_t exePath[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, exePath, &size)) {
            CloseHandle(hProcess);
            return QFileInfo(QString::fromWCharArray(exePath)).fileName();
        }
        CloseHandle(hProcess);
    }
    return QString();
}

struct EnumCtx {
    std::vector<WindowState>* list;
    HWND selfHwnd;
};

BOOL CALLBACK EnumWindowsProcInternal(HWND hwnd, LPARAM lParam) {
    EnumCtx* ctx = reinterpret_cast<EnumCtx*>(lParam);
    if (WindowManager::isUserWindow(hwnd, ctx->selfHwnd)) {
        ctx->list->push_back(WindowManager::captureWindowState(hwnd));
    }
    return TRUE;
}

std::vector<WindowState> WindowManager::getTopLevelWindows(HWND selfHwnd) {
    std::vector<WindowState> result;
    EnumCtx ctx{ &result, selfHwnd };
    EnumWindows(EnumWindowsProcInternal, reinterpret_cast<LPARAM>(&ctx));
    return result;
}

void WindowManager::hideWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) return;
    if (!IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_MINIMIZE);
    }
}

void WindowManager::restoreWindow(HWND hwnd, const WindowState& state) {
    if (!hwnd || !IsWindow(hwnd)) return;

    if (state.isZoomed) {
        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    } else {
        ShowWindow(hwnd, SW_RESTORE);
    }
}

void WindowManager::activateWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) return;

    if (IsIconic(hwnd)) {
        WINDOWPLACEMENT wp{ sizeof(WINDOWPLACEMENT) };
        GetWindowPlacement(hwnd, &wp);
        if (wp.flags & WPF_RESTORETOMAXIMIZED) {
            ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        } else {
            ShowWindow(hwnd, SW_RESTORE);
        }
    } else {
        ShowWindow(hwnd, SW_SHOW);
    }

    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
}
