// RasterImageItem.h
#pragma once

#include <QQuickPaintedItem>
#include <QImage>

class RasterImageItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)

public:
    RasterImageItem(QQuickItem* parent = nullptr);

    QImage image() const;
    void setImage(const QImage& img);

    void paint(QPainter* painter) override;

signals:
    void imageChanged();

private:
    QImage m_image;
};
