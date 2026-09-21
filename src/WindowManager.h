#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#endif

#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QVector>
#include <QObject>
#include <vector>

struct WindowState {
    HWND hwnd = nullptr;
    QString title;
    QString className;
    DWORD processId = 0;
    QString exePath;
    QIcon icon;
    WINDOWPLACEMENT placement{ sizeof(WINDOWPLACEMENT) };
    bool isZoomed = false;
    bool isIconic = false;
    bool isVisible = false;
};

struct AppGroup {
    QString id;
    QString name;
    QIcon icon;
    std::vector<HWND> hwnds;
    DWORD processId = 0;
};

class WindowManager : public QObject {
    Q_OBJECT
public:
    explicit WindowManager(QObject* parent = nullptr);
    ~WindowManager();

    static bool isUserWindow(HWND hwnd, HWND selfHwnd = nullptr);
    static WindowState captureWindowState(HWND hwnd);
    static QIcon getWindowIcon(HWND hwnd);
    static QString getProcessName(DWORD pid);

    std::vector<WindowState> getTopLevelWindows(HWND selfHwnd = nullptr);
    
    void hideWindow(HWND hwnd);
    void restoreWindow(HWND hwnd, const WindowState& state);
    void activateWindow(HWND hwnd);

    void startHook();
    void stopHook();

signals:
    void foregroundWindowChanged(HWND hwnd);
    void windowDestroyedOrHidden(HWND hwnd);

private:
    static void CALLBACK winEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                      LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
    HWINEVENTHOOK m_hook = nullptr;
};

#endif // WINDOW_MANAGER_H
