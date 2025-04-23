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

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    VizData *vizData = nullptr;
    QVector<QVector<SquareItem*>> squareItemList;
    QPushButton *clearButton = nullptr;
    bool isPanning = false;
    QPoint lastMousePos;
    int currentZoomLevel = 0;
    double currentZoomFactor = 1.0;
    int cellSize = 30;

protected slots:
    virtual void onSquareClicked(const QPoint &pos, const QColor &color);

signals:
    void squareClickedOnScene(QPoint colRow, QColor color);
};

#endif // GRAPHICSVIEWBASE_H
