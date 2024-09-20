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
    squareItemList = QVector<QVector<SquareItem*>>();

    setRenderHint(QPainter::Antialiasing, true);  // Optional: improve rendering quality
    setDragMode(QGraphicsView::NoDrag);  // Disable default drag mode
    setInteractive(true);  // Ensure interactivity for panning and zooming
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);  // Force full updates on view changes
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Ensure zooming anchors to the mouse position
    //Set background color to grey
    setBackgroundBrush(QBrush(QColor(0, 0, 0)));
}

void GraphicsViewCustom::setVizData(VizData *vizData){
    this->vizData = vizData;
}

void GraphicsViewCustom::initSquareItems(){
    qDebug() << "Initializing square items" << this->vizData->rasterData->raster->NCOLS << " " << this->vizData->rasterData->raster->NROWS;
    squareItemList = QVector<QVector<SquareItem*>>(vizData->rasterData->raster->NCOLS,
                                                    QVector<SquareItem*>(vizData->rasterData->raster->NROWS,
                                                                          new SquareItem()));
}

void GraphicsViewCustom::initSquareScene(){

    scene()->setBackgroundBrush(Qt::black);

    scene()->clear();

    for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;

        SquareItem *square = new SquareItem(col,row);
        square->setBrush(QColor::fromRgbF(0.1,0.1,0.1));
        QObject::connect(square, &SquareItem::squareClicked, this, &GraphicsViewCustom::onSquareClicked);
        squareItemList[col][row] = square;

        scene()->addItem(square);
    }
    scene()->update();
}

void GraphicsViewCustom::setSceneCustom(QGraphicsScene *scene){
    setScene(scene);
}

void GraphicsViewCustom::resizeEvent(QResizeEvent *event) {
    if(clearButton){
        clearButton->setGeometry(this->width() - clearButton->width() - 10, 10, 80, 40);
    }
    QGraphicsView::resizeEvent(event);
}

// Mouse panning
void GraphicsViewCustom::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        isPanning = true;
        lastMousePos = event->pos(); // Change cursor when panning
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphicsViewCustom::clearSelection()
{
    qDebug() << "Clearing selection";
    if (scene()) {
        for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
            int row = vizData->rasterData->locationPair1DTo2D[loc].first;
            int col = vizData->rasterData->locationPair1DTo2D[loc].second;
            squareItemList[col][row]->setSelection(false);
        }
        clearButton->hide();
        emit squareClickedOnScene(QPoint(-1,-1), QColor(0,0,0));
    }
}

void GraphicsViewCustom::showClearButton(bool show){
    clearButton->setHidden(!show);
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

void GraphicsViewCustom::onSquareClicked(const QPoint &pos, const QColor &color) {
    qDebug() << "[Graphics] Square clicked at: " << pos << "Color: " << color;

    // Check if a square is selected
    if (!clearButton) {
        clearButton = new QPushButton("Clear", this);
        clearButton->setGeometry(this->width() - clearButton->width() - 10, 10, 80, 40);
        //make button white background and white text
        clearButton->setStyleSheet("background-color: white");
        clearButton->setStyleSheet("color: white");
        connect(clearButton, &QPushButton::clicked, this, &GraphicsViewCustom::clearSelection);
        clearButton->show();
    }

    emit squareClickedOnScene(pos,color);
}

void GraphicsViewCustom::resetGraphicsView(){
    initSquareScene();
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

void GraphicsViewCustom::updateRasterDataMedian(const QString colName, int month) {

    if(squareItemList.isEmpty()){
        initSquareItems();
        initSquareScene();
    }

    if(colName.isEmpty()){
        qDebug() << "[GraphicsViewCustom] Column name is empty!";
        return;
    }

    for(int loc = 0; loc < vizData->rasterData->nLocations; loc++){
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;
        double value = vizData->statsData[colName].median[month][loc];

        // Normalize the value to range [0, 1] based on min and max values
        float normalizedValue = (static_cast<float>(value) - vizData->statsData[colName].medianMin) / (vizData->statsData[colName].medianMax - vizData->statsData[colName].medianMin);

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

