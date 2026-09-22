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
    setFixedSize(160, 120);

    m_hoverAnimation = new QPropertyAnimation(this, "hoverScale", this);
    m_hoverAnimation->setDuration(150);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

QSize ThumbnailWidget::sizeHint() const {
    return QSize(160, 120);
}

void ThumbnailWidget::enterEvent(QEnterEvent* event) {
    QWidget::enterEvent(event);
    m_hoverAnimation->stop();
    m_hoverAnimation->setEndValue(1.05);
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

    // Center area for transformed thumbnail
    qreal scale = m_hoverScale;
    int cardW = 140 * scale;
    int cardH = 95 * scale;
    int cardX = (w - cardW) / 2;
    int cardY = (h - cardH) / 2 - 5;

    QRectF cardRect(cardX, cardY, cardW, cardH);

    // Apply 3D perspective transformation
    painter.save();
    painter.translate(cardRect.center());

    QTransform transform;
    // Perspective rotation along Y axis (macOS Stage Manager tilt effect)
    transform.rotate(-10, Qt::YAxis);
    transform.rotate(2, Qt::ZAxis);
    painter.setTransform(transform, true);

    QRectF localRect(-cardW / 2.0, -cardH / 2.0, cardW, cardH);

    // Drop Shadow
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(localRect.translated(4, 6), 10, 10);
    painter.fillPath(shadowPath, QColor(0, 0, 0, 100));

    // Thumbnail Rounded Border Clip Path
    QPainterPath clipPath;
    clipPath.addRoundedRect(localRect, 10, 10);

    // Background fill
    painter.setClipPath(clipPath);
    painter.fillRect(localRect, QColor(30, 30, 35, 230));

    // Render snapshot or mockup
    if (!m_snapshot.isNull()) {
        painter.drawPixmap(localRect.toRect(), m_snapshot);
    } else {
        // Fallback miniature window graphics
        QRectF headerRect(localRect.x(), localRect.y(), localRect.width(), 18);
        painter.fillRect(headerRect, QColor(50, 50, 60, 240));

        // Window control dots (● ● ●)
        painter.setBrush(QColor(255, 95, 86)); painter.drawEllipse(QPointF(localRect.x() + 10, localRect.y() + 9), 3, 3);
        painter.setBrush(QColor(255, 189, 46)); painter.drawEllipse(QPointF(localRect.x() + 18, localRect.y() + 9), 3, 3);
        painter.setBrush(QColor(39, 201, 63));  painter.drawEllipse(QPointF(localRect.x() + 26, localRect.y() + 9), 3, 3);

        // Content lines mockup
        painter.setPen(QColor(140, 140, 160, 180));
        int lineY = localRect.y() + 28;
        for (int i = 0; i < 4; ++i) {
            int lineWidth = (i % 2 == 0) ? cardW * 0.7 : cardW * 0.4;
            painter.drawLine(localRect.x() + 12, lineY, localRect.x() + 12 + lineWidth, lineY);
            lineY += 12;
        }
    }

    // Active state border highlight
    if (m_isActive) {
        painter.setClipping(false);
        QPen activePen(QColor(0, 150, 255, 230), 2.5);
        painter.setPen(activePen);
        painter.drawRoundedRect(localRect, 10, 10);
    } else {
        painter.setClipping(false);
        QPen normPen(QColor(255, 255, 255, 40), 1.0);
        painter.setPen(normPen);
        painter.drawRoundedRect(localRect, 10, 10);
    }

    painter.restore();

    // Draw Application Icon Badge at bottom center/right
    if (!m_icon.isNull()) {
        int iconSize = 24;
        int iconX = cardX + cardW - iconSize + 2;
        int iconY = cardY + cardH - iconSize + 4;
        m_icon.paint(&painter, QRect(iconX, iconY, iconSize, iconSize));
    }

    // Draw window count badge if multiple windows in group
    if (m_windowCount > 1) {
        QRect badgeRect(cardX - 4, cardY + cardH - 18, 20, 20);
        painter.setBrush(QColor(0, 122, 255));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(badgeRect);
        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setPixelSize(10);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(badgeRect, Qt::AlignCenter, QString::number(m_windowCount));
    }
}
