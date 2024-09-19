#include "squareitem.h"

#include <QPen>
#include <QRandomGenerator>

SquareItem::SquareItem(QGraphicsItem *parent) : QGraphicsRectItem(parent){
}

SquareItem::SquareItem(int col, int row, QGraphicsItem *parent) : QGraphicsRectItem(parent){
    squareSize = 25;
    cellSize = 30;
    setRect(col*cellSize, row*cellSize, squareSize, squareSize);
    squareColRow = QPoint(col, row);
    isSelected = false;
    setSelection(isSelected);
    brush = QBrush(Qt::GlobalColor::gray);
}

void SquareItem::setSelection(bool selected){
    isSelected = selected;
    if(isSelected){
        selectedColor = QColor::fromRgb(QRandomGenerator::global()->generate());
        setPen(QPen(selectedColor, 5.0));
    }
    else{
        selectedColor = Qt::black;
        setPen(QPen(Qt::black, 0.5));
    }
}

void SquareItem::setBrushCustom(QBrush brush){
    setBrush(brush);
}

void SquareItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    // qDebug() << "Square clicked at position:" << event->scenePos() << "Col/Row:" << squareColRow;

    if(event->button() == Qt::LeftButton){
        isSelected = !isSelected;  // Toggle the selection state

        setSelection(isSelected);  // Update the visual appearance

        qDebug() << "[Square] select at:" << squareColRow << "color: " << selectedColor;

        emit squareClicked(squareColRow,selectedColor);  // Emit signal with the column and row
    }

    QGraphicsRectItem::mousePressEvent(event);  // Call base class to keep default behavior
}
