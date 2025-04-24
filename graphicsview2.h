#pragma once
#include "graphicsviewbase.h"

class GraphicsView2 : public GraphicsViewBase
{
    Q_OBJECT
public:
    explicit GraphicsView2(QWidget *parent = nullptr);
    Q_INVOKABLE void updateRasterData();
    Q_INVOKABLE void updateRasterDataPixmap();
    Q_INVOKABLE void updateRasterDataFreq(const QString& aa_sequence, int month,
                                                            double thresholdMin,
                                          double thresholdMax);
    Q_INVOKABLE void updateRasterDataFreqPixmap(const QString& aa_sequence, int month);
};
