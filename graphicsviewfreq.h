#pragma once
#include "graphicsviewbase.h"

class GraphicsViewFreq : public GraphicsViewBase
{
    Q_OBJECT
public:
    explicit GraphicsViewFreq(QWidget *parent = nullptr);
    Q_INVOKABLE void updateRasterData();
    Q_INVOKABLE void updateRasterDataFreq(const QString& aa_sequence, int month);
};
