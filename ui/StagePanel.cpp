#include "StagePanel.h"
#include <QGuiApplication>
#include <QScreen>
#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#endif

GroupTileWidget::GroupTileWidget(const AppGroup& group, bool isActive, QWidget* parent)
    : QWidget(parent), m_groupId(group.id), m_isActive(isActive) {
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(80);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(10);

    // App Icon
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setFixedSize(48, 48);
    if (!group.icon.isNull()) {
        iconLabel->setPixmap(group.icon.pixmap(48, 48));
    } else {
        iconLabel->setText("App");
        iconLabel->setAlignment(Qt::AlignCenter);
    }
    layout->addWidget(iconLabel);

    // App Info
    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    textLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* titleLabel = new QLabel(group.name, this);
    titleLabel->setStyleSheet("color: white; font-weight: bold; font-size: 13px;");
    titleLabel->setToolTip(group.name);
    textLayout->addWidget(titleLabel);

    if (group.hwnds.size() > 1) {
        QLabel* countLabel = new QLabel(QString("%1 windows").arg(group.hwnds.size()), this);
        countLabel->setStyleSheet("color: #AAAAAA; font-size: 11px;");
        textLayout->addWidget(countLabel);
    }

    layout->addLayout(textLayout);
    layout->addStretch();

    // Active state styling
    if (m_isActive) {
        setStyleSheet("GroupTileWidget { background-color: rgba(255, 255, 255, 0.25); border-radius: 12px; border: 1px solid rgba(255, 255, 255, 0.4); }"
                      "GroupTileWidget:hover { background-color: rgba(255, 255, 255, 0.35); }");
    } else {
        setStyleSheet("GroupTileWidget { background-color: rgba(40, 40, 40, 0.6); border-radius: 12px; border: 1px solid rgba(255, 255, 255, 0.1); }"
                      "GroupTileWidget:hover { background-color: rgba(70, 70, 70, 0.8); }");
    }
}

void GroupTileWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_groupId);
    }
    QWidget::mousePressEvent(event);
}

void GroupTileWidget::enterEvent(QEnterEvent* event) {
    QWidget::enterEvent(event);
}

void GroupTileWidget::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
}

// StagePanel Implementation
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
    setFixedWidth(200);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 16, 12, 16);

    // Main background card container
    QWidget* container = new QWidget(this);
    container->setObjectName("container");
    container->setStyleSheet("#container { background-color: rgba(20, 20, 20, 0.85); border-radius: 16px; border: 1px solid rgba(255, 255, 255, 0.15); }");

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(8, 12, 8, 12);

    QLabel* headerLabel = new QLabel("Recent Apps", container);
    headerLabel->setStyleSheet("color: #DDDDDD; font-weight: bold; font-size: 14px; margin-bottom: 8px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(headerLabel);

    m_scrollArea = new QScrollArea(container);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; } QWidget { background: transparent; }");

    QWidget* scrollWidget = new QWidget(m_scrollArea);
    m_listLayout = new QVBoxLayout(scrollWidget);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(10);
    m_listLayout->addStretch();

    m_scrollArea->setWidget(scrollWidget);
    containerLayout->addWidget(m_scrollArea);

    mainLayout->addWidget(container);

    // Drop shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setOffset(4, 0);
    container->setGraphicsEffect(shadow);

    m_slideAnimation = new QPropertyAnimation(this, "geometry", this);
    m_slideAnimation->setDuration(250);
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
        int panelHeight = screenGeometry.height() - 80;
        int yPos = screenGeometry.top() + 40;

        m_visibleRect = QRect(screenGeometry.left() + 10, yPos, panelWidth, panelHeight);
        m_hiddenRect = QRect(screenGeometry.left() - panelWidth + 5, yPos, panelWidth, panelHeight);

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
        GroupTileWidget* tile = new GroupTileWidget(group, isActive, m_scrollArea->widget());
        connect(tile, &GroupTileWidget::clicked, this, &StagePanel::onTileClicked);
        m_listLayout->addWidget(tile);
    }

    m_listLayout->addStretch();
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
    bool nearLeftEdge = (globalPos.x() <= screenGeo.left() + 15 &&
                         globalPos.y() >= m_visibleRect.top() &&
                         globalPos.y() <= m_visibleRect.bottom());

    bool mouseOverPanel = m_visibleRect.contains(globalPos);

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
