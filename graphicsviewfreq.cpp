#include "GraphicsViewFreq.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QtConcurrent>

#include "squareitem.h"

GraphicsViewFreq::GraphicsViewFreq(QWidget *parent) {
    isPanning = false;
    lastMousePos = QPoint();
    cellSize = 30;
    vizData = new VizData();
    squareItemList = QVector<QVector<SquareItem*>>();    
    currentZoomLevel = 1.0;
    currentZoomFactor = zoomFactor;

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


void GraphicsViewFreq::updateRasterDataPixmap(){
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

void GraphicsViewFreq::updateRasterDataFreq(const QString& aa_sequence, int month,
                                            double thresholdMin,
                                            double thresholdMax) {
    if (squareItemList.isEmpty()) {
        initSquareItems();
        initSquareScene();
    }

    if(aa_sequence.isEmpty()){
        qDebug() << "[GraphicsViewFreq] Amino acid sequence is empty!";
        return;
    }

    for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;

        double freq = vizData->genotypeFreqMatrix[aa_sequence][month][loc];

        if((thresholdMin != 0.0 && freq < thresholdMin) || (thresholdMax != 0.0 && freq > thresholdMax)){
            squareItemList[col][row]->setBrushCustom(QBrush(QColor::fromRgbF(0.1f, 0.1f, 0.1f)));
            continue;
        }

        // Normalize the value to range [0, 1] based on min and max values
        float normalizedValue = (static_cast<float>(freq) - thresholdMin) / (thresholdMax - thresholdMin);

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


    setOverlayText(QString("%1\nWildtype frequency: %2%\nArtermisinin-Lumenfantrine Resistance frequency: %3%\n")
                       .arg(vizData->simStartDate.addMonths(month).toString("yyyy-MM-dd"))
                       .arg(vizData->statsFrequencySummary["KNF--R1"].median[month], 0, 'f', 2)
                       .arg(vizData->statsFrequencySummary["KNF--H1"].median[month], 0, 'f', 2));


    scene()->invalidate();
}


void GraphicsViewFreq::updateRasterDataFreqPixmap(const QString& aa_sequence, int month) {
    if (aa_sequence.isEmpty()) {
        qDebug() << "[GraphicsViewFreq] Amino acid sequence is empty!";
        return;
    }

    // Prepare data lookup table
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

    // Get min and max frequency
    double minVal = 0.0, maxVal = 1.0;
    if (vizData->genotypeFrequencyRange.contains(aa_sequence)) {
        minVal = vizData->genotypeFrequencyRange[aa_sequence].first;
        maxVal = vizData->genotypeFrequencyRange[aa_sequence].second;
    }

    const int ncols = vizData->rasterData->raster->NCOLS;
    const int nrows = vizData->rasterData->raster->NROWS;

    // Define color mapping function
    auto colorFunc = [=](int row, int col) -> QColor {
        int loc = vizData->rasterData->locationPair2DTo1D.value(QPair<int, int>(row, col), -1);
        double value = locationFreqMap.value(loc, 0.0);  // defaults to 0 if not found

        return computeColorFromValue(
            value,
            minVal,
            maxVal,
            vizData->colorMap,
            [=](int idx, float factor) { return vizData->interpolate(idx, factor); }
            );
    };

    // Async image creation
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
            centerOn(pixmapItem);
        }, Qt::QueuedConnection);
    });
}

