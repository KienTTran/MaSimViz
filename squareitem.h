#ifndef SQUAREITEM_H
#define SQUAREITEM_H

#include <QObject>
#include <QPair>
#include <QBrush>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

class SquareItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    SquareItem(QGraphicsItem *parent = nullptr);
    SquareItem(int col, int row, QGraphicsItem *parent = nullptr);
    void setSelection(bool selected);
    void setBrushCustom(QBrush brush);

protected:
    // Reimplement the mouse press event
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

public:
    QPoint squareColRow;
    QColor selectedColor;
    QBrush brush;
    bool isSelected = false;
    int cellSize;
    int squareSize;
signals:
    void squareClicked(QPoint squareColRow, QColor selectedColor);
};


#endif // SQUAREITEM_H
