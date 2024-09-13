    #include "graphicsviewcustom.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>

GraphicsViewCustom::GraphicsViewCustom(QWidget *parent) {
    isPanning = false;
    lastMousePos = QPoint();
    currentZoomLevel = 1.0;
    currentZoomFactor = 1.0;
    setRenderHint(QPainter::Antialiasing, true);  // Optional: improve rendering quality
    setDragMode(QGraphicsView::NoDrag);  // Disable default drag mode
    setInteractive(true);  // Ensure interactivity for panning and zooming
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);  // Force full updates on view changes
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Ensure zooming anchors to the mouse position
    //Set background color to grey
    setBackgroundBrush(QBrush(QColor(200, 200, 200)));
}


// Mouse panning
void GraphicsViewCustom::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isPanning = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);  // Change cursor when panning
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
    if (event->button() == Qt::LeftButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);  // Reset cursor
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// Zooming with mouse wheel
// void GraphicsViewCustom::wheelEvent(QWheelEvent *event) {
//     const double zoomFactor = 1.01;  // Smaller zoom factor for smooth zoom
//     const int maxZoomLevel = 500;  // Maximum zoom level
//     const int minZoomLevel = -500;  // Minimum zoom level
//     isPanning = false;  // Disable panning while zooming
//     if (event->angleDelta().y() > 0) {
//         // Zoom in
//         if (currentZoomLevel < maxZoomLevel) {
//             currentZoomLevel++;
//             scale(zoomFactor, zoomFactor);
//         }
//     } else {
//         // Zoom out
//         if (currentZoomLevel > minZoomLevel) {
//             currentZoomLevel--;
//             scale(1.0 / zoomFactor, 1.0 / zoomFactor);
//         }
//     }
//     QGraphicsView::wheelEvent(event);
// }

void GraphicsViewCustom::adjustZoomLevel(int zoomLevel){
    // Map slider value (0-100) to a zoom factor between 0.5 (200% zoom out) and 2.0 (200% zoom in)
    double targetZoomFactor = 0.2 + (zoomLevel / 100.0) * 4.8;

    // Reset the transformation matrix to avoid cumulative scaling
    QTransform transform;
    transform.scale(targetZoomFactor, targetZoomFactor);  // Apply the absolute scale
    setTransform(transform);  // Set the new transformation
}


// Function to display .asc data on QGraphicsView as dots
void GraphicsViewCustom::displayAscData(QGraphicsScene *scene, const QList<QList<double>> &gridData, double cellSize) {
    int rows = gridData.size();
    int cols = gridData.isEmpty() ? 0 : gridData[0].size();

    //Find min and max value
    double minValue = 0;
    double maxValue = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double value = gridData[i][j];
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

    //segment the value range to 6 colors: green, blue, yellow, orange, red, purple
    double segment = 6;
    qDebug() << "Min value: " << minValue << " Max value: " << maxValue << " Segment: " << segment;
    QColor colors[] = {QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 255, 0), QColor(255, 165, 0), QColor(255, 0, 0), QColor(128, 0, 128)};

    // Loop through the grid data and draw each value as a dot
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double value = gridData[i][j];

            if (value != -9999) {  // Assuming -9999 is the NODATA_value
                // Scale the dot size (you can adjust the scaling factor)
                double dotSize = 3.0;
                double dotLineWidth = 0.5;

                // Create an ellipse item for the dot
                QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(j * cellSize, i * cellSize, dotSize, dotSize);
                // Set the color based on the cell value
                // int colorIndex = (value - minValue) / segment;
                // dot->setBrush(QBrush(colors[colorIndex]));
                dot->setBrush(Qt::white);
                //set line width
                dot->setPen(QPen(Qt::black, dotLineWidth));

                // Add the dot to the scene
                scene->addItem(dot);
            }
        }
    }
}
