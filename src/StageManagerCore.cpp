#include "StageManagerCore.h"
#include <QDebug>

StageManagerCore::StageManagerCore(WindowManager* winManager, QObject* parent)
    : QObject(parent), m_winManager(winManager) {
    if (m_winManager) {
        connect(m_winManager, &WindowManager::foregroundWindowChanged,
                this, &StageManagerCore::onForegroundWindowChanged);
        connect(m_winManager, &WindowManager::windowDestroyedOrHidden,
                this, &StageManagerCore::onWindowDestroyedOrHidden);
    }
}

StageManagerCore::~StageManagerCore() {}

void StageManagerCore::refreshWindows() {
    if (!m_winManager) return;
    std::vector<WindowState> windows = m_winManager->getTopLevelWindows(m_selfHwnd);

    HWND foregroundHwnd = GetForegroundWindow();

    std::vector<AppGroup> newGroups;

    for (const auto& ws : windows) {
        m_savedStates[ws.hwnd] = ws;

        AppGroup* existingGroup = findGroupContaining(ws.hwnd);
        if (!existingGroup) {
            AppGroup group;
            group.id = QString::number((quintptr)ws.hwnd);
            group.name = ws.title;
            group.icon = ws.icon;
            group.processId = ws.processId;
            group.hwnds.push_back(ws.hwnd);
            newGroups.push_back(group);
        }
    }

    if (newGroups.size() > 0) {
        m_recentGroups = newGroups;
    }

    if (foregroundHwnd && WindowManager::isUserWindow(foregroundHwnd, m_selfHwnd)) {
        updateGroupsAndBackground(foregroundHwnd);
    }

    emit recentGroupsUpdated();
}

AppGroup* StageManagerCore::findGroupContaining(HWND hwnd) {
    for (auto& group : m_recentGroups) {
        for (HWND h : group.hwnds) {
            if (h == hwnd) return &group;
        }
    }
    return nullptr;
}

AppGroup* StageManagerCore::findGroupById(const QString& groupId) {
    for (auto& group : m_recentGroups) {
        if (group.id == groupId) return &group;
    }
    return nullptr;
}

void StageManagerCore::onForegroundWindowChanged(HWND hwnd) {
    if (m_state != StageState::Idle) return;
    if (!WindowManager::isUserWindow(hwnd, m_selfHwnd)) return;

    m_state = StageState::Tracking;
    updateGroupsAndBackground(hwnd);
    m_state = StageState::Idle;
}

void StageManagerCore::onWindowDestroyedOrHidden(HWND hwnd) {
    Q_UNUSED(hwnd);
    if (m_state != StageState::Idle) return;
    refreshWindows();
}

void StageManagerCore::updateGroupsAndBackground(HWND foregroundHwnd) {
    if (!foregroundHwnd || !IsWindow(foregroundHwnd)) return;

    // Capture state of foreground window
    WindowState fgState = WindowManager::captureWindowState(foregroundHwnd);
    m_savedStates[foregroundHwnd] = fgState;

    std::vector<HWND> activeGroupHwnds;
    AppGroup* foundGroup = findGroupContaining(foregroundHwnd);

    if (!foundGroup) {
        AppGroup newGroup;
        newGroup.id = QString::number((quintptr)foregroundHwnd);
        newGroup.name = fgState.title;
        newGroup.icon = fgState.icon;
        newGroup.processId = fgState.processId;
        newGroup.hwnds.push_back(foregroundHwnd);

        m_recentGroups.insert(m_recentGroups.begin(), newGroup);
        m_activeGroupId = newGroup.id;
        activeGroupHwnds = newGroup.hwnds;
    } else {
        m_activeGroupId = foundGroup->id;
        activeGroupHwnds = foundGroup->hwnds;

        // Move active group to top of recent order (MRU) safely by ID
        QString targetId = foundGroup->id;
        auto it = m_recentGroups.begin();
        while (it != m_recentGroups.end()) {
            if (it->id == targetId) {
                AppGroup g = *it;
                m_recentGroups.erase(it);
                m_recentGroups.insert(m_recentGroups.begin(), g);
                break;
            } else {
                ++it;
            }
        }
    }

    emit activeGroupChanged(m_activeGroupId);
    emit recentGroupsUpdated();

    // Minimize all background windows that are NOT in the active group
    m_state = StageState::ManagingBackground;

    std::vector<WindowState> allWindows = m_winManager->getTopLevelWindows(m_selfHwnd);
    for (const auto& ws : allWindows) {
        bool isInActiveGroup = false;
        for (HWND activeHwnd : activeGroupHwnds) {
            if (activeHwnd == ws.hwnd) {
                isInActiveGroup = true;
                break;
            }
        }

        if (!isInActiveGroup && ws.hwnd != foregroundHwnd) {
            if (!m_savedStates.contains(ws.hwnd)) {
                m_savedStates[ws.hwnd] = ws;
            }
            m_winManager->hideWindow(ws.hwnd);
        }
    }

    m_state = StageState::Idle;
}

void StageManagerCore::switchToGroup(const QString& groupId) {
    if (m_state != StageState::Idle) return;

    AppGroup* group = findGroupById(groupId);
    if (!group || group->hwnds.empty()) return;

    m_state = StageState::Switching;

    // Copy HWNDs to local vector before modifying group
    std::vector<HWND> targetHwnds = group->hwnds;

    if (m_switchMode == SwitchMode::AllAtOnce) {
        for (HWND h : targetHwnds) {
            if (m_savedStates.contains(h)) {
                m_winManager->restoreWindow(h, m_savedStates[h]);
            }
            m_winManager->activateWindow(h);
        }
    } else {
        HWND firstHwnd = targetHwnds.front();
        if (m_savedStates.contains(firstHwnd)) {
            m_winManager->restoreWindow(firstHwnd, m_savedStates[firstHwnd]);
        }
        m_winManager->activateWindow(firstHwnd);

        if (group->hwnds.size() > 1) {
            HWND front = group->hwnds.front();
            group->hwnds.erase(group->hwnds.begin());
            group->hwnds.push_back(front);
        }
    }

    HWND activeHwnd = targetHwnds.front();
    updateGroupsAndBackground(activeHwnd);

    m_state = StageState::Idle;
}

void StageManagerCore::switchToWindow(HWND hwnd) {
    if (m_state != StageState::Idle) return;
    m_state = StageState::Switching;

    if (m_savedStates.contains(hwnd)) {
        m_winManager->restoreWindow(hwnd, m_savedStates[hwnd]);
    }
    m_winManager->activateWindow(hwnd);
    updateGroupsAndBackground(hwnd);

    m_state = StageState::Idle;
}

void StageManagerCore::groupWindows(HWND hwndA, HWND hwndB) {
    AppGroup* groupA = findGroupContaining(hwndA);
    AppGroup* groupB = findGroupContaining(hwndB);

    if (groupA && groupB && groupA->id != groupB->id) {
        for (HWND h : groupB->hwnds) {
            groupA->hwnds.push_back(h);
        }

        QString groupBId = groupB->id;
        auto it = m_recentGroups.begin();
        while (it != m_recentGroups.end()) {
            if (it->id == groupBId) {
                m_recentGroups.erase(it);
                break;
            } else {
                ++it;
            }
        }
        emit recentGroupsUpdated();
    }
}

void StageManagerCore::ungroupWindow(HWND hwnd) {
    AppGroup* group = findGroupContaining(hwnd);
    if (group && group->hwnds.size() > 1) {
        auto it = std::find(group->hwnds.begin(), group->hwnds.end(), hwnd);
        if (it != group->hwnds.end()) {
            group->hwnds.erase(it);

            WindowState ws = WindowManager::captureWindowState(hwnd);
            AppGroup newGroup;
            newGroup.id = QString::number((quintptr)hwnd);
            newGroup.name = ws.title;
            newGroup.icon = ws.icon;
            newGroup.processId = ws.processId;
            newGroup.hwnds.push_back(hwnd);

            m_recentGroups.push_back(newGroup);
            emit recentGroupsUpdated();
        }
    }
}
