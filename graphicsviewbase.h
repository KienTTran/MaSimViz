// graphicsviewbase.h
#ifndef GRAPHICSVIEWBASE_H
#define GRAPHICSVIEWBASE_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QVector>
#include <QPoint>
#include <QColor>
#include <QLabel> // Add this at the top
#include "squareitem.h"
#include "vizdata.h"

class GraphicsViewBase : public QGraphicsView {
    Q_OBJECT

public:
    explicit GraphicsViewBase(QWidget *parent = nullptr);

    void setVizData(VizData *vizData);
    void setSceneCustom(QGraphicsScene *scene);
    void adjustZoomLevel(int zoomLevel);
    void initSquareItems();
    void initSquareScene();
    void clearSelection();
    void showClearButton(bool show);
    void resetGraphicsView();
    void setCellSize(int size){
        cellSize = size;
    }
    void setOverlayText(const QString& text);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    QImage createRasterImage(int width, int height, std::function<QColor(int row, int col)> colorFunc);
    QColor computeColorFromValue(float value, float minVal, float maxVal, const QVector<QVector3D>& colorMap,
                                 std::function<QVector3D(int, float)> interpolate);

    VizData *vizData = nullptr;
    QVector<QVector<SquareItem*>> squareItemList;
    QPushButton *clearButton = nullptr;
    bool isPanning;  // Flag to track whether panning is active
    QPoint lastMousePos;  // Last recorded mouse position
    int currentZoomLevel;  // Current zoom level to limit zooming range
    double currentZoomFactor;  // Current zoom factor to adjust the view
    const double zoomFactor = 1.05;  // Smaller zoom factor for smooth zoom
    const int maxZoomLevel = 500;  // Maximum zoom level
    const int minZoomLevel = 0;  // Minimum zoom level
    int cellSize = 30;  // Size of each square cell

    QLabel* overlayLabel = nullptr;

    QGraphicsPixmapItem* pixmapItem = nullptr;

protected slots:
    virtual void onSquareClicked(const QPoint &pos, const QColor &color);

signals:
    void squareClickedOnScene(QPoint colRow, QColor color);
};

#endif // GRAPHICSVIEWBASE_H
