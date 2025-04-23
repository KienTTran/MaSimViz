#pragma once

#include <QQuickImageProvider>
#include <QImage>
#include <vector>

class RasterImageProvider : public QQuickImageProvider {
public:
    RasterImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    // Set from MainWindow or controller
    void setRasterData(const std::vector<float>& data, int width, int height, float minVal, float maxVal);

private:
    std::vector<float> rasterData;
    int width = 0;
    int height = 0;
    float minVal = 0.0f;
    float maxVal = 1.0f;
};
