#include "graphicsviewcustom.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QtConcurrent>

#include "squareitem.h"

GraphicsViewCustom::GraphicsViewCustom(QWidget *parent) {
    isPanning = false;
    lastMousePos = QPoint();
    currentZoomLevel = 1.0;
    currentZoomFactor = 1.0;
    cellSize = 30;
    vizData = new VizData();
    squareItemList = QVector<QVector<SquareItem*>>();

    setRenderHint(QPainter::Antialiasing, true);  // Optional: improve rendering quality
    setDragMode(QGraphicsView::NoDrag);  // Disable default drag mode
    setInteractive(true);  // Ensure interactivity for panning and zooming
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);  // Force full updates on view changes
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Ensure zooming anchors to the mouse position
    //Set background color to grey
    setBackgroundBrush(QBrush(QColor(0, 0, 0)));
}

// Function to display .asc data on QGraphicsView as dots
void GraphicsViewCustom::updateRasterData() {

    if(squareItemList.isEmpty()){
        initSquareItems();
        initSquareScene();
    }

    for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;
        double value = vizData->rasterData->raster->data[row][col];

        // Normalize the value to range [0, 1] based on min and max values
        float normalizedValue = (static_cast<float>(value) - vizData->rasterData->dataMin) / (vizData->rasterData->dataMax - vizData->rasterData->dataMin);

        // Determine which color stop range this value falls into
        int nColorSteps = vizData->colorMap.size() - 1;
        float stepSize = 1.0f / nColorSteps;
        int lowerStep = qFloor(normalizedValue / stepSize);
        float factor = (normalizedValue - lowerStep * stepSize) / stepSize;

        // Ensure we don't go out of bounds
        if (lowerStep >= nColorSteps) {
            lowerStep = nColorSteps - 1;
            factor = 1.0f;
        }

        // Interpolate between the two adjacent colors
        QVector3D color = vizData->interpolate(lowerStep, factor);

        squareItemList[col][row]->setBrushCustom(QBrush(QColor::fromRgbF(color.x(), color.y(), color.z())));
    }
    scene()->update();
}

void GraphicsViewCustom::updateRasterDataPixmap(){
    int ncols = vizData->rasterData->raster->NCOLS;
    int nrows = vizData->rasterData->raster->NROWS;

    auto colorFunc = [&](int row, int col) -> QColor {
        double value = vizData->rasterData->raster->data[row][col];
        float normalized = (value - vizData->rasterData->dataMin) / (vizData->rasterData->dataMax - vizData->rasterData->dataMin);
        int nSteps = vizData->colorMap.size() - 1;
        float stepSize = 1.0f / nSteps;
        int lowerStep = qFloor(normalized / stepSize);
        float factor = (normalized - lowerStep * stepSize) / stepSize;
        if (lowerStep >= nSteps) { lowerStep = nSteps - 1; factor = 1.0f; }
        QVector3D color = vizData->interpolate(lowerStep, factor);
        return QColor::fromRgbF(color.x(), color.y(), color.z());
    };

    QImage image = createRasterImage(ncols, nrows, colorFunc);
    QPixmap pixmap = QPixmap::fromImage(image);

    scene()->clear();
    if (!pixmapItem) pixmapItem = new QGraphicsPixmapItem();
    pixmapItem->setPixmap(pixmap);
    scene()->addItem(pixmapItem);
}

void GraphicsViewCustom::updateRasterDataMedian(const QString colName, int month) {

    if(squareItemList.isEmpty()){
        initSquareItems();
        initSquareScene();
    }

    if(colName.isEmpty()){
        qDebug() << "[GraphicsViewCustom] Column name is empty!";
        return;
    }

    int row = -1;
    int col = -1;
    double value = 0.0;
    for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
        row = vizData->rasterData->locationPair1DTo2D[loc].first;
        col = vizData->rasterData->locationPair1DTo2D[loc].second;
        if(vizData->isDistrictReporter){
            int dictrictLoc = vizData->rasterData->locationPair2DTo1DDistrict[QPair<int,int>(row,col)];
            value = vizData->statsData[colName].iqr[0][month][dictrictLoc];
        }
        else{
            value = vizData->statsData[colName].iqr[0][month][loc];
        }

        // Normalize the value to range [0, 1] based on min and max values
        float normalizedValue = (static_cast<float>(value) - vizData->statsData[colName].medianMin) / (vizData->statsData[colName].medianMax - vizData->statsData[colName].medianMin);
        // float normalizedValue = (static_cast<float>(value) - rasterData->dataMin) / (rasterData->dataMax - rasterData->dataMin);

        // Determine which color stop range this value falls into
        int nColorSteps = vizData->colorMap.size() - 1;
        float stepSize = 1.0f / nColorSteps;
        int lowerStep = qFloor(normalizedValue / stepSize);
        float factor = (normalizedValue - lowerStep * stepSize) / stepSize;

        // Ensure we don't go out of bounds
        if (lowerStep >= nColorSteps) {
            lowerStep = nColorSteps - 1;
            factor = 1.0f;
        }

        // Interpolate between the two adjacent colors
        QVector3D color = vizData->interpolate(lowerStep, factor);

        QColor newColor = QColor::fromRgbF(color.x(), color.y(), color.z());
        if (squareItemList[col][row]->brush.color() != newColor) {
            squareItemList[col][row]->setBrushCustom(QBrush(newColor));
        }

    }
    scene()->invalidate();
}

void GraphicsViewCustom::updateRasterDataMedianPixmap(const QString colName, int month) {
    if (colName.isEmpty()) {
        qDebug() << "[GraphicsViewCustom] Column name is empty!";
        return;
    }

    if (!vizData || !vizData->rasterData || !vizData->rasterData->raster) {
        qWarning() << "[GraphicsViewCustom] Invalid vizData or raster";
        return;
    }

    const int ncols = vizData->rasterData->raster->NCOLS;
    const int nrows = vizData->rasterData->raster->NROWS;

    if (ncols <= 0 || nrows <= 0) {
        qWarning() << "[GraphicsViewCustom] Invalid raster size:" << ncols << "x" << nrows;
        return;
    }

    // Capture by value for threading safety
    auto colorFunc = [=](int row, int col) -> QColor {
        double value = 0.0;
        if (vizData->isDistrictReporter) {
            int districtLoc = vizData->rasterData->locationPair2DTo1DDistrict.value(QPair<int, int>(row, col), -1);
            if (districtLoc >= 0) {
                value = vizData->statsData[colName].iqr[0][month][districtLoc];
            }
        } else {
            int loc = vizData->rasterData->locationPair2DTo1D.value(QPair<int, int>(row, col), -1);
            if (loc >= 0) {
                value = vizData->statsData[colName].iqr[0][month][loc];
            }
        }

        return computeColorFromValue(
            value,
            vizData->statsData[colName].medianMin,
            vizData->statsData[colName].medianMax,
            vizData->colorMap,
            [=](int idx, float factor) { return vizData->interpolate(idx, factor); }
            );
    };

    // Run image rendering in background
    QtConcurrent::run([=]() {
        QImage image = createRasterImage(nrows, ncols, colorFunc);
        QPixmap pixmap = QPixmap::fromImage(image);

        QMetaObject::invokeMethod(this, [=]() {
            if (!scene()) setScene(new QGraphicsScene(this));

            if (!pixmapItem) {
                pixmapItem = new QGraphicsPixmapItem();
                scene()->addItem(pixmapItem);
            }

            pixmapItem->setPixmap(pixmap);
            scene()->setSceneRect(0, 0, image.width(), image.height());
            centerOn(pixmapItem); // or: fitInView(...)
        }, Qt::QueuedConnection);
    });
}
