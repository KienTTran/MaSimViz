#ifndef CHARTCUSTOM_H
#define CHARTCUSTOM_H

#include <QObject>

#include <QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "vizdata.h"

class ChartCustom : public QObject
{
    Q_OBJECT
public:
    explicit ChartCustom(QObject *parent = nullptr);

private:
    QChart *chart;
    QGraphicsLineItem *verticalLine = nullptr;
    QList<QGraphicsSimpleTextItem*> valueLabels;

public slots:
    void plotDataMedianMultipleLocations(QChartView* chartView, VizData *vizData, int colIndex, QMap<int,QColor> locInfo, int currentMonth, QString title);
signals:
    void valueAtVerticalLineChanged(double value);
};

#endif // CHARTCUSTOM_H
