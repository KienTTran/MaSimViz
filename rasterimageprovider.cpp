
// RasterImageProvider.cpp
#include "RasterImageProvider.h"

RasterImageProvider::RasterImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image) {}

void RasterImageProvider::setImage(const QImage &image) {
    QMutexLocker locker(&mutex_);
    image_ = image.copy(); // Copy to avoid lifetime issues
}

QImage RasterImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    QMutexLocker locker(&mutex_);
    if (size)
        *size = image_.size();
    return image_;
}
