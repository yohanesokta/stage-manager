#include "ThumbnailWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

ThumbnailWidget::ThumbnailWidget(const AppGroup& group, const QPixmap& snapshot, bool isActive, QWidget* parent)
    : QWidget(parent), m_groupId(group.id), m_title(group.name), m_icon(group.icon),
      m_snapshot(snapshot), m_isActive(isActive), m_windowCount((int)group.hwnds.size()) {

    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFixedSize(160, 110);

    m_hoverAnimation = new QPropertyAnimation(this, "hoverScale", this);
    m_hoverAnimation->setDuration(150);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

QSize ThumbnailWidget::sizeHint() const {
    return QSize(160, 110);
}

void ThumbnailWidget::enterEvent(QEnterEvent* event) {
    QWidget::enterEvent(event);
    m_hoverAnimation->stop();
    m_hoverAnimation->setEndValue(1.06);
    m_hoverAnimation->start();
}

void ThumbnailWidget::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
    m_hoverAnimation->stop();
    m_hoverAnimation->setEndValue(1.0);
    m_hoverAnimation->start();
}

void ThumbnailWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_groupId);
    }
    QWidget::mousePressEvent(event);
}

void ThumbnailWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    int w = width();
    int h = height();

    qreal scale = m_hoverScale;
    int baseW = 125 * scale;
    int baseH = 80 * scale;

    // Center horizontally, push slightly up for bottom icon
    int mainX = (w - baseW) / 2 + 6;
    int mainY = (h - baseH) / 2 - 4;

    // Render stacked group windows pushing backwards in 3D depth if multiple windows
    int stackCount = qMin(m_windowCount, 3);
    for (int i = stackCount - 1; i >= 1; --i) {
        // Offset stacked background windows pushing up/right into depth
        int offsetX = i * 6;
        int offsetY = -i * 6;
        qreal depthScale = 1.0 - (i * 0.05);

        int backW = baseW * depthScale;
        int backH = baseH * depthScale;
        int backX = mainX + offsetX;
        int backY = mainY + offsetY;

        QRectF backRect(backX, backY, backW, backH);

        QPainterPath backPath;
        backPath.addRoundedRect(backRect, 8, 8);

        // Soft shadow for stacked card
        QPainterPath backShadow;
        backShadow.addRoundedRect(backRect.translated(2, 3), 8, 8);
        painter.fillPath(backShadow, QColor(0, 0, 0, 40));

        painter.fillPath(backPath, QColor(220, 225, 235, 210));
        QPen stackPen(QColor(0, 0, 0, 40), 1.0);
        painter.setPen(stackPen);
        painter.drawRoundedRect(backRect, 8, 8);
    }

    // Main Window Thumbnail Card
    QRectF cardRect(mainX, mainY, baseW, baseH);

    // Soft Drop Shadow floating over desktop
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(cardRect.translated(2, 4), 9, 9);
    painter.fillPath(shadowPath, QColor(0, 0, 0, 90));

    // Thumbnail Rounded Border Clip Path
    QPainterPath clipPath;
    clipPath.addRoundedRect(cardRect, 9, 9);

    painter.save();
    painter.setClipPath(clipPath);

    // Fill window background
    painter.fillRect(cardRect, QColor(240, 243, 246, 245));

    // Render Window Snapshot or fallback UI mockup
    if (!m_snapshot.isNull()) {
        painter.drawPixmap(cardRect.toRect(), m_snapshot);
    } else {
        // Window Titlebar header
        QRectF headerRect(cardRect.x(), cardRect.y(), cardRect.width(), 16);
        painter.fillRect(headerRect, QColor(225, 228, 232, 250));

        // Window Control Dots (● ● ●)
        painter.setBrush(QColor(255, 95, 86)); painter.drawEllipse(QPointF(cardRect.x() + 8, cardRect.y() + 8), 2.5, 2.5);
        painter.setBrush(QColor(255, 189, 46)); painter.drawEllipse(QPointF(cardRect.x() + 15, cardRect.y() + 8), 2.5, 2.5);
        painter.setBrush(QColor(39, 201, 63));  painter.drawEllipse(QPointF(cardRect.x() + 22, cardRect.y() + 8), 2.5, 2.5);

        // Content lines mockup
        painter.setPen(QColor(180, 185, 195));
        int lineY = cardRect.y() + 24;
        for (int i = 0; i < 3; ++i) {
            int lineW = (i % 2 == 0) ? baseW * 0.65 : baseW * 0.4;
            painter.drawLine(cardRect.x() + 10, lineY, cardRect.x() + 10 + lineW, lineY);
            lineY += 10;
        }
    }
    painter.restore();

    // Card Border Highlight
    if (m_isActive) {
        QPen activePen(QColor(0, 122, 255, 240), 2.0);
        painter.setPen(activePen);
        painter.drawRoundedRect(cardRect, 9, 9);
    } else {
        QPen normPen(QColor(0, 0, 0, 35), 1.0);
        painter.setPen(normPen);
        painter.drawRoundedRect(cardRect, 9, 9);
    }

    // Draw Application Icon at BOTTOM-LEFT corner (overlapping bottom-left of thumbnail)
    if (!m_icon.isNull()) {
        int iconSize = 28;
        int iconX = mainX - 8;
        int iconY = mainY + baseH - iconSize + 6;

        // Soft shadow under icon
        painter.setBrush(QColor(0, 0, 0, 50));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(iconX + 1, iconY + 2, iconSize, iconSize);

        m_icon.paint(&painter, QRect(iconX, iconY, iconSize, iconSize));
    }
}
