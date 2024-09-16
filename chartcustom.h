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
    QLineSeries *series;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QLineSeries *currentVerticalLine = nullptr;
    QGraphicsSimpleTextItem *valueLabel = nullptr;

public slots:
    void plotDataMedian1Location(QChartView* chartView, VizData *vizData, int colIndex, int locIndex, QString title);
    void plotVerticalLineOnChart(VizData *vizData, int colIndex, int locIndex, int month);
signals:
    void valueAtVerticalLineChanged(double value);
};

#endif // CHARTCUSTOM_H
