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
    scene()->clear();
    scene()->setBackgroundBrush(Qt::black);
    initSquareItems();
    scene()->update();
    centerOn(scene()->sceneRect().center());
    scale(0.35, 0.35);
}

void GraphicsViewBase::resizeEvent(QResizeEvent *event) {
    if (clearButton) {
        clearButton->setGeometry(this->width() - clearButton->width() - 10, 10, 80, 40);
    }
    QGraphicsView::resizeEvent(event);
}

void GraphicsViewBase::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        isPanning = true;
        lastMousePos = event->pos();
    }
    QGraphicsView::mousePressEvent(event);
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
    if (event->button() == Qt::RightButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsViewBase::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        QGraphicsView::wheelEvent(event);
    } else {
        const double zoomFactor = 1.05;
        const int maxZoomLevel = 500;
        const int minZoomLevel = -500;
        if (event->angleDelta().y() > 0 && currentZoomLevel < maxZoomLevel) {
            currentZoomLevel++;
            scale(zoomFactor, zoomFactor);
        } else if (event->angleDelta().y() < 0 && currentZoomLevel > minZoomLevel) {
            currentZoomLevel--;
            scale(1.0 / zoomFactor, 1.0 / zoomFactor);
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
    if (!clearButton) {
        clearButton = new QPushButton("Clear", this);
        clearButton->setGeometry(this->width() - clearButton->width() - 10, 10, 80, 40);
        clearButton->setStyleSheet("background-color: white; color: white");
        connect(clearButton, &QPushButton::clicked, this, &GraphicsViewBase::clearSelection);
        clearButton->show();
    }
    emit squareClickedOnScene(pos, color);
}
