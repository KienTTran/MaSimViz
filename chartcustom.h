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
    VizData *vizData;
    QChart *chart;
    QGraphicsLineItem *verticalLine = nullptr;
    QList<QGraphicsSimpleTextItem*> valueLabels;
    QChartView *chartView;

public:
    void setChartView(QChartView* chartView);
    void setVizData(VizData *vizData);
public slots:
    void plotDataMedianMultipleLocations(QString colName, QMap<QPair<int,int>,QColor> locInfo, int currentMonth, QString title);
signals:
    void valueAtVerticalLineChanged(double value);
};

#endif // CHARTCUSTOM_H
