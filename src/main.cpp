#include "main.h"
#include "WindowManager.h"
#include "StageManagerCore.h"
#include "../ui/StagePanel.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Stage Manager for Windows");
    app.setQuitOnLastWindowClosed(false);

    // Create native Win32 window manager
    WindowManager winManager;

    // Create core stage manager engine
    StageManagerCore stageCore(&winManager);

    // Create Qt UI panel widget
    StagePanel stagePanel(&stageCore);
    stagePanel.show();

    // Set self HWND so Stage Manager excludes itself from window tracking
    stageCore.setSelfHwnd(stagePanel.getHwnd());

    // Start native Win32 event hook
    winManager.startHook();

    // Initial window scan
    stageCore.refreshWindows();

    qDebug() << "Stage Manager for Windows started successfully.";

    return app.exec();
}