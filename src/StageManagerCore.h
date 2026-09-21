#ifndef STAGE_MANAGER_CORE_H
#define STAGE_MANAGER_CORE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include "WindowManager.h"

enum class StageState {
    Idle,
    Tracking,
    Switching,
    Restoring,
    ManagingBackground
};

enum class SwitchMode {
    AllAtOnce,
    OneAtTime
};

class StageManagerCore : public QObject {
    Q_OBJECT
public:
    explicit StageManagerCore(WindowManager* winManager, QObject* parent = nullptr);
    ~StageManagerCore();

    void setSelfHwnd(HWND hwnd) { m_selfHwnd = hwnd; }

    void refreshWindows();
    void switchToGroup(const QString& groupId);
    void switchToWindow(HWND hwnd);

    void groupWindows(HWND hwndA, HWND hwndB);
    void ungroupWindow(HWND hwnd);

    const std::vector<AppGroup>& getRecentGroups() const { return m_recentGroups; }
    QString getActiveGroupId() const { return m_activeGroupId; }

    SwitchMode getSwitchMode() const { return m_switchMode; }
    void setSwitchMode(SwitchMode mode) { m_switchMode = mode; }

signals:
    void recentGroupsUpdated();
    void activeGroupChanged(const QString& groupId);

private slots:
    void onForegroundWindowChanged(HWND hwnd);
    void onWindowDestroyedOrHidden(HWND hwnd);

private:
    void updateGroupsAndBackground(HWND foregroundHwnd);
    AppGroup* findGroupContaining(HWND hwnd);
    AppGroup* findGroupById(const QString& groupId);

    WindowManager* m_winManager = nullptr;
    HWND m_selfHwnd = nullptr;

    std::vector<AppGroup> m_recentGroups;
    QMap<HWND, WindowState> m_savedStates;
    QString m_activeGroupId;

    StageState m_state = StageState::Idle;
    SwitchMode m_switchMode = SwitchMode::AllAtOnce;
};

#endif // STAGE_MANAGER_CORE_H
