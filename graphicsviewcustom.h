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
    // void wheelEvent(QWheelEvent *event) override;

public:
    void adjustZoomLevel(int zoomLevel);
    void displayAscData(QGraphicsScene *scene, const QList<QList<double>> &gridData, double cellSize);
private:
    bool isPanning;  // Flag to track whether panning is active
    QPoint lastMousePos;  // Last recorded mouse position
    int currentZoomLevel;  // Current zoom level to limit zooming range
    double currentZoomFactor;  // Current zoom factor to adjust the view
};

#endif // GRAPHICSVIEWCUSTOM_H
