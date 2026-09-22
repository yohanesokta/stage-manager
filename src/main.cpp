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
    WindowManager winManager;
    StageManagerCore stageCore(&winManager);
    StagePanel stagePanel(&stageCore);
    stagePanel.show();
    stageCore.setSelfHwnd(stagePanel.getHwnd());
    winManager.startHook();
    stageCore.refreshWindows();
    qDebug() << "Stage Manager for Windows started successfully.";
    return app.exec();
}