#include "StagePanel.h"
#include <QGuiApplication>
#include <QScreen>
#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#endif

StagePanel::StagePanel(StageManagerCore* core, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      m_core(core) {

    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    setupUi();

    if (m_core) {
        connect(m_core, &StageManagerCore::recentGroupsUpdated, this, &StagePanel::updateRecentGroups);
        connect(m_core, &StageManagerCore::activeGroupChanged, this, &StagePanel::updateActiveGroup);
    }

    m_edgeTimer = new QTimer(this);
    connect(m_edgeTimer, &QTimer::timeout, this, &StagePanel::checkMousePosition);
    m_edgeTimer->start(100);
}

StagePanel::~StagePanel() {}

HWND StagePanel::getHwnd() const {
    return (HWND)winId();
}

void StagePanel::setupUi() {
    setFixedWidth(175);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Completely transparent container floating over desktop
    QWidget* container = new QWidget(this);
    container->setObjectName("container");
    container->setStyleSheet("#container { background: transparent; border: none; }");

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(container);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; } QWidget { background: transparent; }");

    QWidget* scrollWidget = new QWidget(m_scrollArea);
    m_listLayout = new QVBoxLayout(scrollWidget);
    m_listLayout->setContentsMargins(0, 5, 0, 35);
    m_listLayout->setSpacing(12);

    m_scrollArea->setWidget(scrollWidget);
    containerLayout->addWidget(m_scrollArea);

    mainLayout->addWidget(container);

    m_slideAnimation = new QPropertyAnimation(this, "geometry", this);
    m_slideAnimation->setDuration(220);
    m_slideAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void StagePanel::setupNativeWindowFlags() {
#ifdef _WIN32
    HWND hwnd = getHwnd();
    if (hwnd) {
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        // WS_EX_NOACTIVATE ensures clicking panel does not steal focus from active app window!
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW);
    }
#endif
}

void StagePanel::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    setupNativeWindowFlags();
    updatePanelPosition();
}

void StagePanel::updatePanelPosition() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int panelWidth = width();
        int panelHeight = screenGeometry.height() - 40;
        int yPos = screenGeometry.top() + 20;

        m_visibleRect = QRect(screenGeometry.left() + 2, yPos, panelWidth, panelHeight);
        m_hiddenRect = QRect(screenGeometry.left() - panelWidth + 2, yPos, panelWidth, panelHeight);

        if (m_isShown) {
            setGeometry(m_visibleRect);
        } else {
            setGeometry(m_hiddenRect);
        }
    }
}

void StagePanel::updateRecentGroups() {
    if (!m_core) return;

    // Clear layout
    QLayoutItem* item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    const auto& groups = m_core->getRecentGroups();
    QString activeId = m_core->getActiveGroupId();

    for (const auto& group : groups) {
        bool isActive = (group.id == activeId);

        // Fetch or capture window snapshot pixmap
        QPixmap snapshot;
        if (!group.hwnds.empty()) {
            HWND h = group.hwnds.front();
            if (isActive || !m_snapshotCache.contains(h)) {
                QPixmap cap = WindowManager::captureWindowSnapshot(h, QSize(260, 180));
                if (!cap.isNull()) {
                    m_snapshotCache[h] = cap;
                }
            }
            if (m_snapshotCache.contains(h)) {
                snapshot = m_snapshotCache[h];
            }
        }

        ThumbnailWidget* tile = new ThumbnailWidget(group, snapshot, isActive, m_scrollArea->widget());
        connect(tile, &ThumbnailWidget::clicked, this, &StagePanel::onTileClicked);
        m_listLayout->addWidget(tile);
    }

    // Add padding space at bottom of scroll list so scrolling reaches bottom-most item completely
    m_listLayout->addSpacing(35);
}

void StagePanel::updateActiveGroup(const QString& groupId) {
    Q_UNUSED(groupId);
    updateRecentGroups();
}

void StagePanel::onTileClicked(const QString& groupId) {
    if (m_core) {
        QTimer::singleShot(0, this, [this, groupId]() {
            if (m_core) {
                m_core->switchToGroup(groupId);
            }
        });
    }
}

void StagePanel::checkMousePosition() {
    if (!m_isAutoHide) return;

    QPoint globalPos = QCursor::pos();
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect screenGeo = screen->availableGeometry();

    // Check if foreground active window touches/overlaps the left UI area
    bool windowOverlapsUi = false;
#ifdef _WIN32
    HWND fgHwnd = GetForegroundWindow();
    if (fgHwnd && WindowManager::isUserWindow(fgHwnd, getHwnd())) {
        RECT rc;
        if (GetWindowRect(fgHwnd, &rc) && !IsIconic(fgHwnd)) {
            QRect fgRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            if (fgRect.intersects(m_visibleRect)) {
                windowOverlapsUi = true;
            }
        }
    }
#endif

    bool nearLeftEdge = (globalPos.x() <= screenGeo.left() + 20 &&
                         globalPos.y() >= m_visibleRect.top() &&
                         globalPos.y() <= m_visibleRect.bottom());

    bool mouseOverPanel = m_visibleRect.contains(globalPos);

    if (!windowOverlapsUi) {
        // If NO window touches the Stage Manager UI area -> ALWAYS KEEP VISIBLE!
        if (!m_isShown) {
            setPanelVisible(true);
        }
    } else {
        // ONLY if a window overlaps/covers the Stage Manager UI area -> AUTO-HIDE unless hovered!
        if (nearLeftEdge || mouseOverPanel) {
            if (!m_isShown) {
                setPanelVisible(true);
            }
        } else {
            if (m_isShown) {
                setPanelVisible(false);
            }
        }
    }
}

void StagePanel::setPanelVisible(bool visible) {
    if (m_isShown == visible) return;
    m_isShown = visible;

    m_slideAnimation->stop();
    m_slideAnimation->setStartValue(geometry());
    m_slideAnimation->setEndValue(visible ? m_visibleRect : m_hiddenRect);
    m_slideAnimation->start();
}

void StagePanel::enterEvent(QEnterEvent* event) {
    QWidget::enterEvent(event);
    if (m_isAutoHide && !m_isShown) {
        setPanelVisible(true);
    }
}

void StagePanel::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
}
