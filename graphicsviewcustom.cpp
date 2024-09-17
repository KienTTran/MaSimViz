#include "graphicsviewcustom.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>

#include "squareitem.h"
#include "mainwindow.h"

GraphicsViewCustom::GraphicsViewCustom(QWidget *parent) {
    isPanning = false;
    lastMousePos = QPoint();
    currentZoomLevel = 1.0;
    currentZoomFactor = 1.0;
    cellSize = 30;
    vizData = new VizData();
    selectedSquareList = QVector<QVector<bool>>();
    selectedSquareColorList = QVector<QVector<QColor>>();

    setRenderHint(QPainter::Antialiasing, true);  // Optional: improve rendering quality
    setDragMode(QGraphicsView::NoDrag);  // Disable default drag mode
    setInteractive(true);  // Ensure interactivity for panning and zooming
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);  // Force full updates on view changes
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Ensure zooming anchors to the mouse position
    //Set background color to grey
    setBackgroundBrush(QBrush(QColor(0, 0, 0)));
}


// Mouse panning
void GraphicsViewCustom::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        isPanning = true;
        lastMousePos = event->pos(); // Change cursor when panning
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphicsViewCustom::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        // Get the difference in mouse movement
        QPoint delta = event->pos() - lastMousePos;

        // Adjust the scrollbars by the delta (move the view)
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        // Update the lastMousePos to the current position
        lastMousePos = event->pos();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsViewCustom::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);  // Reset cursor
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// Zooming with mouse wheel
void GraphicsViewCustom::wheelEvent(QWheelEvent *event) {

    // your functionality, for example:
    // if ctrl pressed, use original functionality
    if (event->modifiers() & Qt::ControlModifier)
    {
        QGraphicsView::wheelEvent(event);
    }
    // otherwise, do yours
    else
    {
        const double zoomFactor = 1.05;  // Smaller zoom factor for smooth zoom
        const int maxZoomLevel = 500;  // Maximum zoom level
        const int minZoomLevel = -500;  // Minimum zoom level
        isPanning = false;  // Disable panning while zooming
        if (event->angleDelta().y() > 0) {
            // Zoom in
            if (currentZoomLevel < maxZoomLevel) {
                currentZoomLevel++;
                scale(zoomFactor, zoomFactor);
            }
        } else {
            // Zoom out
            if (currentZoomLevel > minZoomLevel) {
                currentZoomLevel--;
                scale(1.0 / zoomFactor, 1.0 / zoomFactor);
            }
        }
    }
}

void GraphicsViewCustom::adjustZoomLevel(int zoomLevel){
    // Map slider value (0-100) to a zoom factor between 0.5 (200% zoom out) and 2.0 (200% zoom in)
    double targetZoomFactor = 0.2 + (zoomLevel / 100.0) * 4.8;

    // Reset the transformation matrix to avoid cumulative scaling
    QTransform transform;
    transform.scale(targetZoomFactor, targetZoomFactor);  // Apply the absolute scale
    setTransform(transform);  // Set the new transformation
}

// Function to display .asc data on QGraphicsView as dots
void GraphicsViewCustom::displayAscData(QGraphicsScene *scene, VizData *vizData) {

    ncols = vizData->rasterData->raster->NCOLS;
    nrows = vizData->rasterData->raster->NROWS;

    //Find min and max value
    double minValue = 0;
    double maxValue = 0;
    for (int i = 0; i < vizData->rasterData->raster->NROWS; ++i) {
        for (int j = 0; j < vizData->rasterData->raster->NCOLS; ++j) {
            double value = vizData->rasterData->raster->data[i][j];
            if (value != -9999) {  // Assuming -9999 is the NODATA_value
                if(value < minValue){
                    minValue = value;
                }
                if(value > maxValue){
                    maxValue = value;
                }
            }
        }
    }

    selectedSquareList = QVector<QVector<bool>>(vizData->rasterData->raster->NCOLS,
                                                QVector<bool>(vizData->rasterData->raster->NROWS,false));

    selectedSquareColorList = QVector<QVector<QColor>>(vizData->rasterData->raster->NCOLS,
                                                QVector<QColor>(vizData->rasterData->raster->NROWS,QColor(0,0,0)));

    //segment the value range to 6 colors: green, blue, yellow, orange, red, purple
    double segment = 6;
    qDebug() << "Min value: " << minValue << " Max value: " << maxValue << " Segment: " << segment;
    QColor colors[] = {QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 255, 0), QColor(255, 165, 0), QColor(255, 0, 0), QColor(128, 0, 128)};

    scene->setBackgroundBrush(Qt::black);

    scene->clear();  // Clear the scene before drawing new data
    // Loop through the grid data and draw each value as a dot

    for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;
        double value = vizData->rasterData->raster->data[row][col];

        // Scale the square size (you can adjust the scaling factor)
        double squareSize = 25.0;
        double squareLineWidth = 0.5;

        // Create a rectangle item for the square
        SquareItem *square = new SquareItem(col,row,col * cellSize, row * cellSize, squareSize, squareSize,
                                            selectedSquareList[col][row],selectedSquareColorList[col][row]);
        QObject::connect(square, &SquareItem::squareClicked, this, &GraphicsViewCustom::onSquareClicked);

        // Normalize the value to range [0, 1] based on min and max values
        float normalizedValue = (static_cast<float>(value) - minValue) / (maxValue - minValue);

        // Determine which color stop range this value falls into
        int nColorSteps = vizData->colorStops.size() - 1;
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

        square->setBrush(QBrush(QColor::fromRgbF(color.x(), color.y(), color.z())));

        // Add the dot to the scene
        scene->addItem(square);
    }
    scene->update();
}

void GraphicsViewCustom::onSquareClicked(const QPoint &pos, const QColor &color) {
    qDebug() << "[Graphics] Square clicked at: " << pos << "Color: " << color;
    selectedSquareList[pos.x()][pos.y()] = !selectedSquareList[pos.x()][pos.y()];
    selectedSquareColorList[pos.x()][pos.y()] = color;
    emit squareClickedOnScene(pos,color);
}

void GraphicsViewCustom::resetGraphicsView(){
    // Reset the transformation matrix to avoid cumulative scaling
    if(selectedSquareList.size() > 0){
        for(int i = 0; i < selectedSquareList.size(); i++){
            for(int j = 0; j < selectedSquareList[i].size(); j++){
                selectedSquareList[i][j] = false;
                selectedSquareColorList[i][j] = QColor(0,0,0);
            }
        }
    }
}

void GraphicsViewCustom::displayAscDataMedian(QGraphicsScene *scene, const int colIndex, VizData *vizData, int month) {
    // qDebug() << "Displaying median data for column: " << colIndex << " and month: " << month;

    scene->setBackgroundBrush(Qt::black);

    scene->clear();  // Clear the scene before drawing new data

    for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;
        double value = vizData->statsData[colIndex].median[month][loc];
        // Scale the square size (you can adjust the scaling factor)
        double squareSize = 25.0;
        double squareLineWidth = 0.5;

        SquareItem *square = new SquareItem(col,row,col * cellSize, row * cellSize, squareSize, squareSize,
                                            selectedSquareList[col][row],selectedSquareColorList[col][row]);
        // Create a rectangle item for the square
        QObject::connect(square, &SquareItem::squareClicked, this, &GraphicsViewCustom::onSquareClicked);

        // Normalize the value to range [0, 1] based on min and max values
        float normalizedValue = (static_cast<float>(value) - vizData->statsData[colIndex].medianMin) / (vizData->statsData[colIndex].medianMax - vizData->statsData[colIndex].medianMin);

        // Determine which color stop range this value falls into
        int nColorSteps = vizData->colorStops.size() - 1;
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

        square->setBrush(QBrush(QColor::fromRgbF(color.x(), color.y(), color.z())));

        // Add the square to the scene
        scene->addItem(square);

    }
    scene->update();
}

