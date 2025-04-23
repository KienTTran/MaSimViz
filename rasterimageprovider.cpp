#include "rasterimageprovider.h"
#include <QtMath>

RasterImageProvider::RasterImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{}

void RasterImageProvider::setRasterData(const std::vector<float>& data, int w, int h, float minV, float maxV) {
    rasterData = data;
    width = w;
    height = h;
    minVal = minV;
    maxVal = maxV;
}

QImage RasterImageProvider::requestImage(const QString &, QSize *size, const QSize &) {
    QImage img(width, height, QImage::Format_Grayscale8);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            float val = rasterData[index];
            float norm = qBound(0.0f, (val - minVal) / (maxVal - minVal), 1.0f);
            int gray = static_cast<int>(norm * 255);
            img.setPixelColor(x, y, QColor(gray, gray, gray));
        }
    }

    if (size)
        *size = QSize(width, height);
    return img;
}
