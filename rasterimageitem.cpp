// RasterImageItem.cpp
#include "RasterImageItem.h"
#include <QPainter>

RasterImageItem::RasterImageItem(QQuickItem* parent)
    : QQuickPaintedItem(parent) {
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setAntialiasing(true);
}

QImage RasterImageItem::image() const {
    return m_image;
}

void RasterImageItem::setImage(const QImage& img) {
    if (m_image != img) {
        m_image = img;
        update();
        emit imageChanged();
    }
}

void RasterImageItem::paint(QPainter* painter) {
    if (!m_image.isNull()) {
        painter->drawImage(boundingRect(), m_image);
    }
}
