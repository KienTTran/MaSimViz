// RasterImageProvider.h
#ifndef RASTERIMAGEPROVIDER_H
#define RASTERIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>

class RasterImageProvider : public QQuickImageProvider {
public:
    RasterImageProvider();

    // Thread-safe setter
    void setImage(const QImage &image);

    // Provide image to QML
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QImage image_;
    QMutex mutex_;
};

#endif // RASTERIMAGEPROVIDER_H
