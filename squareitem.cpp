#include "squareitem.h"

#include <QPen>
#include <QRandomGenerator>

SquareItem::SquareItem(int col, int row, double x, double y, double width, double height, bool selected, QColor color, QGraphicsItem *parent) : QGraphicsRectItem(parent){
    setRect(x, y, width, height);
    squareColRow = QPoint(col, row);
    isSelected = selected;
    selectedColor = color;
    if(isSelected){
        setPen(QPen(selectedColor, 5.0));
    }
    else{
        setPen(QPen(Qt::black, 0.5));
    }
}

void SquareItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    // qDebug() << "Square clicked at position:" << event->scenePos() << "Col/Row:" << squareColRow;

    if(event->button() == Qt::LeftButton){
        isSelected = !isSelected;  // Toggle the selection state

        if(isSelected){
            selectedColor = QColor::fromRgb(QRandomGenerator::global()->generate());
            setPen(QPen(selectedColor, 5.0));
        }
        else{
            selectedColor = Qt::black;
            setPen(QPen(Qt::black, 0.5));
        }

        qDebug() << "[Square] select at:" << squareColRow << "color: " << selectedColor;

        emit squareClicked(squareColRow,selectedColor);  // Emit signal with the column and row
    }

    QGraphicsRectItem::mousePressEvent(event);  // Call base class to keep default behavior
}
