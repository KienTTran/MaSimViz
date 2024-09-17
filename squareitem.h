#ifndef SQUAREITEM_H
#define SQUAREITEM_H

#include <QObject>
#include <QPair>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

class SquareItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    SquareItem(int col, int row, double x, double y, double width, double height, bool selected, QColor color, QGraphicsItem *parent = nullptr);

protected:
    // Reimplement the mouse press event
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override ;

public:
    QPoint squareColRow;
    QColor selectedColor;
    bool isSelected = false;
signals:
    void squareClicked(QPoint squareColRow, QColor selectedColor);
};


#endif // SQUAREITEM_H
