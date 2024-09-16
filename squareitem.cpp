#include "squareitem.h"

#include <QPen>

void SquareItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    // qDebug() << "Square clicked at position:" << event->scenePos() << "Col/Row:" << squareColRow;

    isSelected = !isSelected;  // Toggle the selection state

    if(isSelected){
        setPen(QPen(Qt::red, 3.0));
    }
    else{
        setPen(QPen(Qt::black, 0.5));
    }

    emit squareClicked(squareColRow);  // Emit signal with the column and row

    QGraphicsRectItem::mousePressEvent(event);  // Call base class to keep default behavior
}
