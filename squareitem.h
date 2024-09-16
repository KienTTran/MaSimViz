#ifndef SQUAREITEM_H
#define SQUAREITEM_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

class SquareItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    SquareItem(int col, int row, double x, double y, double width, double height, QGraphicsItem *parent = nullptr)
        : QGraphicsRectItem(x, y, width, height, parent) {
        squareColRow = QPointF(col, row);
    }

protected:
    // Reimplement the mouse press event
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override ;

public:
    QPointF squareColRow;
    bool isSelected = false;
signals:
    void squareClicked(QPointF colRow);
};


#endif // SQUAREITEM_H
