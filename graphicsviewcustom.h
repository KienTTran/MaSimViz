#pragma once
#include "graphicsviewbase.h"

class GraphicsViewCustom : public GraphicsViewBase
{
    Q_OBJECT
public:
    explicit GraphicsViewCustom(QWidget *parent = nullptr);
    Q_INVOKABLE void updateRasterData();
    Q_INVOKABLE void updateRasterDataMedian(const QString colName, int month);
};
