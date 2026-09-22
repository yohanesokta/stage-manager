#ifndef THUMBNAIL_WIDGET_H
#define THUMBNAIL_WIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QIcon>
#include <QTransform>
#include <QPropertyAnimation>
#include "../src/WindowManager.h"

class ThumbnailWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal hoverScale READ hoverScale WRITE setHoverScale)

public:
    explicit ThumbnailWidget(const AppGroup& group, const QPixmap& snapshot, bool isActive, QWidget* parent = nullptr);

    QString groupId() const { return m_groupId; }
    qreal hoverScale() const { return m_hoverScale; }
    void setHoverScale(qreal scale) { m_hoverScale = scale; update(); }

signals:
    void clicked(const QString& groupId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    QSize sizeHint() const override;

private:
    QString m_groupId;
    QString m_title;
    QIcon m_icon;
    QPixmap m_snapshot;
    bool m_isActive = false;
    int m_windowCount = 1;

    qreal m_hoverScale = 1.0;
    QPropertyAnimation* m_hoverAnimation = nullptr;
};

#endif // THUMBNAIL_WIDGET_H
