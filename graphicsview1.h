#ifndef GRAPHICSVIEW1_H
#define GRAPHICSVIEW1_H

#include "graphicsviewbase.h"

class GraphicsView1 : public GraphicsViewBase
{
    Q_OBJECT
public:
    explicit GraphicsView1(QWidget *parent = nullptr);
    Q_INVOKABLE void updateRasterData();
    Q_INVOKABLE void updateRasterDataPixmap();
    Q_INVOKABLE void updateRasterDataFreq(const QString& aa_sequence, int month,
                                          double thresholdMin,
                                          double thresholdMax);
    Q_INVOKABLE void updateRasterDataFreqPixmap(const QString& aa_sequence, int month);
};

#endif // GRAPHICSVIEW1_H
