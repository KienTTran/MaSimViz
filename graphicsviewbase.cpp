// graphicsviewbase.cpp
#include "graphicsviewbase.h"
#include <QScrollBar>
#include <QDebug>

GraphicsViewBase::GraphicsViewBase(QWidget *parent) : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setInteractive(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setBackgroundBrush(QBrush(QColor(0, 0, 0)));

    overlayLabel = new QLabel("Text Overlay", this);
    overlayLabel->setStyleSheet("color: white; background-color: rgba(0, 0, 0, 128); padding: 4px;");
    overlayLabel->setFont(QFont("Arial", 14, QFont::Bold));
    overlayLabel->move(10, height() - 30); // Start at bottom-left
    overlayLabel->show();

}

void GraphicsViewBase::setVizData(VizData *vizData) {
    this->vizData = vizData;
}

void GraphicsViewBase::setSceneCustom(QGraphicsScene *scene) {
    setScene(scene);
}

void GraphicsViewBase::adjustZoomLevel(int zoomLevel) {
    double targetZoomFactor = 0.2 + (zoomLevel / 100.0) * 4.8;
    QTransform transform;
    transform.scale(targetZoomFactor, targetZoomFactor);
    setTransform(transform);
}

void GraphicsViewBase::initSquareItems() {

    int ncols = vizData->rasterData->raster->NCOLS;
    int nrows = vizData->rasterData->raster->NROWS;

    if (ncols <= 0 || nrows <= 0) {
        qWarning() << "[GraphicsViewBase] Invalid raster size: cols =" << ncols << ", rows =" << nrows;
        return;
    }

    squareItemList.clear();
    squareItemList.resize(vizData->rasterData->raster->NCOLS);
    for (int col = 0; col < vizData->rasterData->raster->NCOLS; ++col) {
        squareItemList[col].resize(vizData->rasterData->raster->NROWS, nullptr);
    }

    for (int loc = 0; loc < vizData->rasterData->nLocations; ++loc) {
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;
        SquareItem* square = new SquareItem(col, row, -1);
        square->setBrush(QColor::fromRgbF(0.1,0.1,0.1));
        QObject::connect(square, &SquareItem::squareClicked, this, &GraphicsViewBase::onSquareClicked);
        squareItemList[col][row] = square;
        scene()->addItem(square);
    }
}


void GraphicsViewBase::initSquareScene() {
    //set camera to center and zoom out a bit
    currentZoomLevel = 1.0;
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    centerOn(scene()->sceneRect().center());
    ensureVisible(scene()->sceneRect());
    scene()->update();
    qDebug() << "Init square scene";
}

void GraphicsViewBase::resizeEvent(QResizeEvent *event) {
    if (clearButton) {
        clearButton->setGeometry(this->width() - clearButton->width() - 10, 10, 80, 40);
    }
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    centerOn(scene()->sceneRect().center());
    ensureVisible(scene()->sceneRect());
    scene()->update();

    // Update text position
    if (overlayLabel) {
        overlayLabel->move(10, height() - overlayLabel->height() - 10); // Bottom-left
    }

    QGraphicsView::resizeEvent(event);
}

void GraphicsViewBase::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isPanning = true;
        lastMousePos = event->pos();
    }
    QGraphicsView::mousePressEvent(event);

    // QPointF scenePos = mapToScene(event->pos());
    // int col = static_cast<int>(scenePos.x()) / cellSize;
    // int row = static_cast<int>(scenePos.y()) / cellSize;
    // emit squareClickedOnScene(QPoint(col, row), QColor()); // Placeholder for color
}

void GraphicsViewBase::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        QPoint delta = event->pos() - lastMousePos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        lastMousePos = event->pos();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsViewBase::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsViewBase::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        QGraphicsView::wheelEvent(event);
    } else {

        isPanning = false;  // Disable panning while zooming
        if (event->angleDelta().y() > 0) {
            // Zoom in
            if (currentZoomLevel < maxZoomLevel) {
                currentZoomLevel++;
                scale(currentZoomFactor, currentZoomFactor);
            }
        } else {
            // Zoom out
            if (currentZoomLevel > minZoomLevel) {
                currentZoomLevel--;
                scale(1.0 / currentZoomFactor, 1.0 / currentZoomFactor);
            }
        }
    }
}

void GraphicsViewBase::clearSelection() {
    if (scene()) {
        for (int loc = 0; loc < vizData->rasterData->nLocations; ++loc) {
            int row = vizData->rasterData->locationPair1DTo2D[loc].first;
            int col = vizData->rasterData->locationPair1DTo2D[loc].second;
            squareItemList[col][row]->setSelection(false);
        }
        if (clearButton) clearButton->hide();
        emit squareClickedOnScene(QPoint(-1,-1), QColor(0,0,0));
    }
}

void GraphicsViewBase::showClearButton(bool show) {
    if (clearButton) {
        clearButton->setHidden(!show);
    }
}

void GraphicsViewBase::resetGraphicsView() {
    initSquareScene();
}

void GraphicsViewBase::onSquareClicked(const QPoint &pos, const QColor &color) {
    // if (!clearButton) {
    //     clearButton = new QPushButton("Clear", this);
    //     clearButton->setGeometry(this->width() - clearButton->width() - 10, 10, 80, 40);
    //     clearButton->setStyleSheet("background-color: white; color: white");
    //     connect(clearButton, &QPushButton::clicked, this, &GraphicsViewBase::clearSelection);
    //     clearButton->show();
    // }
    // emit squareClickedOnScene(pos, color);
}

QImage GraphicsViewBase::createRasterImage(
    int rows, int cols, std::function<QColor(int row, int col)> colorFunc)
{
    QImage image(cols * cellSize, rows * cellSize, QImage::Format_ARGB32);
    image.fill(Qt::black);

    QPainter painter(&image);
    for (int loc = 0; loc < vizData->rasterData->nLocations; ++loc) {
        int row = vizData->rasterData->locationPair1DTo2D[loc].first;
        int col = vizData->rasterData->locationPair1DTo2D[loc].second;

        if (loc == 0) {
            QColor color = colorFunc(row, col);
            // qDebug() << "[DEBUG] color at (0,0):" << color;
        }

        painter.fillRect(col * cellSize, row * cellSize, cellSize, cellSize, colorFunc(row, col));
    }

    return image;
}


QColor GraphicsViewBase::computeColorFromValue(
    float value, float minVal, float maxVal,
    const QVector<QVector3D>& colorMap,
    std::function<QVector3D(int, float)> interpolate) {

    float normalized = (maxVal > minVal) ? (value - minVal) / (maxVal - minVal) : 0.0f;
    int nSteps = colorMap.size() - 1;
    float stepSize = 1.0f / nSteps;
    int lowerStep = qFloor(normalized / stepSize);
    float factor = (normalized - lowerStep * stepSize) / stepSize;

    if (lowerStep >= nSteps) {
        lowerStep = nSteps - 1;
        factor = 1.0f;
    }

    QVector3D colorVec = interpolate(lowerStep, factor);
    return QColor::fromRgbF(colorVec.x(), colorVec.y(), colorVec.z());
}

void GraphicsViewBase::setOverlayText(const QString& text) {
    if (overlayLabel) {
        overlayLabel->setText(text);
        int margin = 10;
        int labelWidth = overlayLabel->sizeHint().width();
        int labelHeight = overlayLabel->sizeHint().height();
        overlayLabel->setGeometry(margin, height() - margin - labelHeight, labelWidth, labelHeight);
        overlayLabel->setWordWrap(true);
        overlayLabel->setFixedWidth(250);  // or dynamic based on window

    }
}





