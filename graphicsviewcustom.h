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
#include <QPushButton>

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

    void resizeEvent(QResizeEvent *event) override;

public:
    VizData *vizData;
    void setVizData(VizData *vizData);
    void setSceneCustom(QGraphicsScene *scene);
    void adjustZoomLevel(int zoomLevel);
    void initSquareItems();
    Q_INVOKABLE void updateRasterData();
    Q_INVOKABLE void updateRasterDataMedian(const QString colName, int month);
private:
    bool isPanning;  // Flag to track whether panning is active
    QPoint lastMousePos;  // Last recorded mouse position
    int currentZoomLevel;  // Current zoom level to limit zooming range
    double currentZoomFactor;  // Current zoom factor to adjust the view
    int cellSize;
    QVector<QVector<SquareItem*>> squareItemList;
    QPushButton *clearButton = nullptr;

public slots:
    void onSquareClicked(const QPoint &pos, const QColor &color);
    void resetGraphicsView();
    void clearSelection();
    void initSquareScene();
    void showClearButton(bool show);

signals:
    void squareClickedOnScene(QPoint colRow, QColor color);
};

#endif // GRAPHICSVIEWCUSTOM_H
