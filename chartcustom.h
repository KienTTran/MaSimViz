// In chartcustom.h
#ifndef CHARTCUSTOM_H
#define CHARTCUSTOM_H

#include <QObject>
#include <QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

#include "vizdata.h"
#include "chartbase.h"

class ChartCustom : public ChartBase
{
    Q_OBJECT
public:
    explicit ChartCustom(QObject *parent = nullptr);
};

#endif // CHARTCUSTOM_H
