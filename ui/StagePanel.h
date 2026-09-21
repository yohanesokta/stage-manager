#ifndef STAGE_PANEL_H
#define STAGE_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "../src/StageManagerCore.h"

class GroupTileWidget : public QWidget {
    Q_OBJECT
public:
    explicit GroupTileWidget(const AppGroup& group, bool isActive, QWidget* parent = nullptr);

    QString groupId() const { return m_groupId; }

signals:
    void clicked(const QString& groupId);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString m_groupId;
    bool m_isActive = false;
};

class StagePanel : public QWidget {
    Q_OBJECT
public:
    explicit StagePanel(StageManagerCore* core, QWidget* parent = nullptr);
    ~StagePanel();

    HWND getHwnd() const;

public slots:
    void updateRecentGroups();
    void updateActiveGroup(const QString& groupId);

protected:
    void showEvent(QShowEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void checkMousePosition();
    void onTileClicked(const QString& groupId);

private:
    void setupUi();
    void setupNativeWindowFlags();
    void updatePanelPosition();
    void setPanelVisible(bool visible);

    StageManagerCore* m_core = nullptr;
    QVBoxLayout* m_listLayout = nullptr;
    QScrollArea* m_scrollArea = nullptr;

    QTimer* m_edgeTimer = nullptr;
    bool m_isAutoHide = true;
    bool m_isShown = true;

    QPropertyAnimation* m_slideAnimation = nullptr;
    QRect m_visibleRect;
    QRect m_hiddenRect;
};

#endif // STAGE_PANEL_H
