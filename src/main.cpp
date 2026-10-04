#include "main.h"
#include <windows.h>
#include "WindowManager.h"
#include "StageManagerCore.h"
#include "../ui/StagePanel.h"
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int argc = 0;
    char** argv = nullptr;
    QApplication app(argc, argv);
    app.setApplicationName("Stage Manager for Windows");
    app.setQuitOnLastWindowClosed(false);

    QIcon appIcon(":/icons.ico");
    if (appIcon.isNull()) {
        appIcon = QIcon("icons.ico");
    }
    app.setWindowIcon(appIcon);

    WindowManager winManager;
    StageManagerCore stageCore(&winManager);
    StagePanel stagePanel(&stageCore);
    stagePanel.setWindowIcon(appIcon);
    stagePanel.show();
    stageCore.setSelfHwnd(stagePanel.getHwnd());
    winManager.startHook();
    stageCore.refreshWindows();

    // Setup System Tray Icon
    QSystemTrayIcon trayIcon(appIcon, &app);
    trayIcon.setToolTip("Stage Manager for Windows");

    QMenu* trayMenu = new QMenu();
    QAction* toggleAction = trayMenu->addAction("Show/Hide Stage Manager");
    trayMenu->addSeparator();
    QAction* quitAction = trayMenu->addAction("Quit");

    QObject::connect(toggleAction, &QAction::triggered, [&stagePanel]() {
        if (stagePanel.isVisible()) {
            stagePanel.hide();
        } else {
            stagePanel.show();
            stagePanel.raise();
        }
    });

    QObject::connect(quitAction, &QAction::triggered, [&]() {
        winManager.stopHook();
        stageCore.restoreAllWindows();
        app.quit();
    });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        winManager.stopHook();
        stageCore.restoreAllWindows();
    });

    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [&stagePanel](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            if (stagePanel.isVisible()) {
                stagePanel.hide();
            } else {
                stagePanel.show();
                stagePanel.raise();
            }
        }
    });

    trayIcon.setContextMenu(trayMenu);
    trayIcon.show();

    qDebug() << "Stage Manager for Windows started successfully.";
    return app.exec();
}