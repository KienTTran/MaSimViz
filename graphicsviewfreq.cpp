#include "GraphicsViewFreq.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>

#include "squareitem.h"

GraphicsViewFreq::GraphicsViewFreq(QWidget *parent) {
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
void GraphicsViewFreq::updateRasterData() {

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

void GraphicsViewFreq::updateRasterDataFreq(const QString& aa_sequence, int month) {
    if (squareItemList.isEmpty()) {
        initSquareItems();
        initSquareScene();
    }

    if (aa_sequence.isEmpty()) {
        qDebug() << "[GraphicsViewFreq] Amino acid sequence is empty!";
        return;
    }

    // Create a lookup map: locationid → frequency
    QMap<int, double> locationFreqMap;
    for (const auto& item : vizData->genotypeFrequencies) {
        if (item.aa_sequence == aa_sequence && item.monthlydataid == month) {
            locationFreqMap[item.locationid] = item.frequency;
        }
    }

    if (locationFreqMap.isEmpty()) {
        qDebug() << "[GraphicsViewFreq] No genotype frequency found for month:" << month << ", aa_sequence:" << aa_sequence;
        return;
    }

    // Determine min and max values for normalization
    double minVal = 0.0;
    double maxVal = 1.0;
    if (vizData->genotypeFrequencyRange.contains(aa_sequence)) {
        minVal = vizData->genotypeFrequencyRange[aa_sequence].first;
        maxVal = vizData->genotypeFrequencyRange[aa_sequence].second;
    }


    int row = -1;
    int col = -1;
    for (int loc = 0; loc < vizData->rasterData->nLocations; ++loc) {
        row = vizData->rasterData->locationPair1DTo2D[loc].first;
        col = vizData->rasterData->locationPair1DTo2D[loc].second;

        double value = locationFreqMap.value(loc, 0.0);  // default to 0 if not present

        // Normalize the value to range [0, 1]
        float normalizedValue = (maxVal > minVal)
                                    ? (static_cast<float>(value) - minVal) / (maxVal - minVal)
                                    : 0.0f;

        int nColorSteps = vizData->colorMap.size() - 1;
        float stepSize = 1.0f / nColorSteps;
        int lowerStep = qFloor(normalizedValue / stepSize);
        float factor = (normalizedValue - lowerStep * stepSize) / stepSize;

        if (lowerStep >= vizData->colorMap.size() - 1) {
            lowerStep = vizData->colorMap.size() - 2;
            factor = 1.0f;
        }


        QVector3D color = vizData->interpolate(lowerStep, factor);
        QColor newColor = QColor::fromRgbF(color.x(), color.y(), color.z());
        if (squareItemList[col][row]->brush.color() != newColor) {
            squareItemList[col][row]->setBrushCustom(QBrush(newColor));
        }
    }

    scene()->update();
}
