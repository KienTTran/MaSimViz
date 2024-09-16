#ifndef GRAPHICSVIEWCUSTOM_H
#define GRAPHICSVIEWCUSTOM_H
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

#include <vizdata.h>
#include "squareitem.h"

class GraphicsViewCustom : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsViewCustom(QWidget *parent);

    // Mouse panning
    void mousePressEvent(QMouseEvent *event)  override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    // Zooming with mouse wheel
    void wheelEvent(QWheelEvent *event) override;

public:
    VizData *vizData;
    void adjustZoomLevel(int zoomLevel);
    Q_INVOKABLE void displayAscData(QGraphicsScene *scene, VizData *vizData);
    Q_INVOKABLE void displayAscDataMedian(QGraphicsScene *scene,  const int colIndex, VizData *vizData, int month);
private:
    bool isPanning;  // Flag to track whether panning is active
    QPoint lastMousePos;  // Last recorded mouse position
    int currentZoomLevel;  // Current zoom level to limit zooming range
    double currentZoomFactor;  // Current zoom factor to adjust the view
    int cellSize;
    int ncols;
    int nrows;
    QVector<QVector<bool>> selectedSquareList;

public slots:
    void onSquareClicked(const QPointF &pos);
    void resetGraphicsView();

signals:
    void squareClickedOnScene(QPointF colRow);
};

#endif // GRAPHICSVIEWCUSTOM_H
