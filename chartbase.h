#ifndef CHARTBASE_H
#define CHARTBASE_H

#include <QObject>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QScatterSeries>
#include "vizdata.h"

class ChartBase : public QObject {
    Q_OBJECT
public:
    explicit ChartBase(QObject *parent = nullptr);

protected:
    QChart *chart = nullptr;
    QChartView *chartView = nullptr;
    QCategoryAxis* axisX = nullptr;
    QValueAxis* axisY = nullptr;
    bool axesInitialized = false;
    VizData *vizData;
    QGraphicsLineItem *verticalLine = nullptr;
    QList<QGraphicsSimpleTextItem*> valueLabels;
    void setupAxes(int monthCount, QPair<double,double> yMinMax);
private:
    QLineSeries* medianSeries = nullptr;
    QLineSeries* iqr25Series = nullptr;
    QLineSeries* iqr75Series = nullptr;
    QAreaSeries* areaSeries = nullptr;
    QString lastLabel;


public:
    void setupChart(const QString &title);
    void setVizData(VizData *vizData);
    void setChartView(QChartView *view);
    void plotSummaryData(const QMap<QString, VizData::StatsDataSummary>& summaryData,
                                    const QPair<double,double>& yMinMax,
                                    const QString& colName,
                                    int currentMonth,
                         const QString& title);
    void updateVerticalLine(const QMap<QString, VizData::StatsDataSummary>& summaryData,
                            const QString& colName,int currentMonth);
};

#endif // CHARTBASE_H
