#pragma once
#include "graphicsviewbase.h"

class GraphicsView : public GraphicsViewBase
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget *parent = nullptr);
    Q_INVOKABLE void updateRasterData();
    Q_INVOKABLE void updateRasterDataPixmap();
    Q_INVOKABLE void updateRasterDataMedian(const QString colName, int month);
    Q_INVOKABLE void updateRasterDataMedianPixmap(const QString colName, int month);
};
