
#include <QRandomGenerator>
#include <QAreaSeries>
#include <QLegendMarker>
#include <QCategoryAxis>
#include <QTimer>

#include "chartbase.h"

ChartBase::ChartBase(QObject *parent)
    : QObject(parent) {
}


void ChartBase::setVizData(VizData *vizData){
    this->vizData = vizData;
}

void ChartBase::setChartView(QChartView *view) {
    chart = new QChart();
    chartView = view;
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    if (chartView && chartView->chart()) {
        chartView->chart()->setBackgroundBrush(QBrush(QColor(0.1, 0.1, 0.1)));
        chartView->chart()->setPlotAreaBackgroundBrush(QBrush(QColor(0.1, 0.1, 0.1)));
        chartView->chart()->setPlotAreaBackgroundVisible(true);
    }
}

void ChartBase::setupChart(const QString &title) {
    chart->removeAllSeries();
    if (!chart) {
        chart = new QChart();
        chartView->setChart(chart);
        chart->setTheme(QChart::ChartThemeDark);
        chart->setBackgroundBrush(QBrush(Qt::black));
        chart->setPlotAreaBackgroundBrush(QBrush(Qt::black));
        chart->setPlotAreaBackgroundVisible(true);
    } else {
        chart->setTitle(title);
    }

}

void ChartBase::setupAxes(int monthCount, QPair<double,double> yMinMax) {
    if (!axesInitialized) {
        axisX = new QCategoryAxis;
        axisX->setRange(0, monthCount - 1);
        for (int i = 0; i <= monthCount; i += 12) {
            axisX->append((i == 0 || i % 60 == 0) ? QString("Year %1").arg(i / 12) : "", i);
        }
        axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
        chart->addAxis(axisX, Qt::AlignBottom);

        axisY = new QValueAxis;
        axisY->setRange(yMinMax.first, yMinMax.second);
        axisY->setLabelFormat("%.2f");
        chart->addAxis(axisY, Qt::AlignRight);

        axesInitialized = true;
    }
}
void ChartBase::plotSummaryData(const QMap<QString, VizData::StatsDataSummary>& summaryData,
                                const QPair<double, double>& yMinMax,
                                const QString& colName,
                                int currentMonth,
                                const QString& title) {
    if (!summaryData.contains(colName) || summaryData[colName].median.isEmpty()) {
        qWarning() << "[ChartCustom] Summary stats not found for column:" << colName;
        return;
    }

    const auto& summary = summaryData[colName];
    int monthCount = summary.median.size();

    if (!chart) {
        chart = new QChart();
        chartView->setChart(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chart->setTheme(QChart::ChartThemeDark);
        chart->setBackgroundBrush(QBrush(Qt::black));
        chart->setPlotAreaBackgroundBrush(QBrush(Qt::black));
        chart->setPlotAreaBackgroundVisible(true);
        chart->setTitleBrush(QBrush(Qt::white));

    }
    chart->setTitle(title);
    chart->setTitleBrush(QBrush(Qt::white));

    if (!axisX || !axisY) {
        axisX = new QCategoryAxis;
        axisX->setRange(0, monthCount - 1);
        for (int i = 0; i <= monthCount; i += 12) {
            axisX->append((i == 0 || i % 60 == 0) ? QString("Year %1").arg(i / 12) : "", i);
        }
        axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
        chart->addAxis(axisX, Qt::AlignBottom);

        axisY = new QValueAxis;
        axisY->setLabelFormat("%.2f");
        chart->addAxis(axisY, Qt::AlignRight);
        axisX->setLabelsBrush(QBrush(Qt::white));
        axisY->setLabelsBrush(QBrush(Qt::white));

    }

    if (!medianSeries) {
        medianSeries = new QLineSeries();
        iqr25Series = new QLineSeries();
        iqr75Series = new QLineSeries();
        areaSeries = new QAreaSeries(iqr25Series, iqr75Series);

        QPen pen(Qt::cyan, 3);
        medianSeries->setPen(pen);
        QColor fillColor = Qt::cyan;
        fillColor.setAlphaF(0.4);
        areaSeries->setBrush(QBrush(fillColor));
        areaSeries->setPen(Qt::NoPen);

        chart->addSeries(areaSeries);
        chart->addSeries(medianSeries);
        areaSeries->attachAxis(axisX);
        areaSeries->attachAxis(axisY);
        medianSeries->attachAxis(axisX);
        medianSeries->attachAxis(axisY);

        for (QLegendMarker* marker : chart->legend()->markers(areaSeries)) {
            marker->setVisible(false);
        }
    }

    // Prepare series data
    QVector<QPointF> medianPoints, iqr25Points, iqr75Points;
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();

    for (int month = 0; month < monthCount; ++month) {
        double med = summary.median[month];
        double p25 = summary.iqr25[month];
        double p75 = summary.iqr75[month];

        medianPoints.append(QPointF(month, med));
        iqr25Points.append(QPointF(month, p25));
        iqr75Points.append(QPointF(month, p75));

        minY = qMin(minY, p25);
        maxY = qMax(maxY, p75);
    }

    medianSeries->replace(medianPoints);
    iqr25Series->replace(iqr25Points);
    iqr75Series->replace(iqr75Points);

    double newMinY = qMin(yMinMax.first, minY);
    double newMaxY = qMin(yMinMax.second, maxY);

    if (axisY->min() != newMinY || axisY->max() != newMaxY) {
        axisY->setRange(newMinY, newMaxY);
        qDebug() << "[ChartBase] Y axis updated to:" << newMinY << "->" << newMaxY;
    }


    // ✅ Update medianSeries name (legend) based on currentMonth
    if (currentMonth >= 0 && currentMonth < monthCount) {
        QString label = QString("Median: %1 (%2 - %3)")
        .arg(QString::number(summary.median[currentMonth], 'f', 2))
            .arg(QString::number(summary.iqr25[currentMonth], 'f', 2))
            .arg(QString::number(summary.iqr75[currentMonth], 'f', 2));

        qDebug() << "[ChartBase] Legend label updated:" << label;

        medianSeries->setName(label);  // ✅ Force update always
        for (QLegendMarker* marker : chart->legend()->markers()) {
            marker->setLabelBrush(QBrush(Qt::white));
        }

    }


    // ✅ Move vertical line
    if (verticalLine) {
        chart->scene()->removeItem(verticalLine);
        delete verticalLine;
        verticalLine = nullptr;
    }

    QPointF top = chart->mapToPosition(QPointF(currentMonth, axisY->max()));
    QPointF bottom = chart->mapToPosition(QPointF(currentMonth, axisY->min()));
    verticalLine = new QGraphicsLineItem(QLineF(bottom, top));
    QPen vPen(Qt::red, 2);
    verticalLine->setPen(vPen);
    chart->scene()->addItem(verticalLine);

    chart->update();
}


void ChartBase::updateVerticalLine(const QMap<QString, VizData::StatsDataSummary>& summaryData,
                                   const QString& colName,
                                   int currentMonth) {
    if (!chart || !axisY || !axisX || !medianSeries) return;

    // 1) Update the series name & legend marker label
    if (summaryData.contains(colName)) {
        const auto& summary = summaryData[colName];
        if (currentMonth >= 0 && currentMonth < summary.median.size()) {
            QString label = QString("Median: %1 (%2 - %3)")
            .arg(QString::number(summary.median[currentMonth], 'f', 2))
                .arg(QString::number(summary.iqr25[currentMonth],    'f', 2))
                .arg(QString::number(summary.iqr75[currentMonth],    'f', 2));

            // Update the series name (internal)…
            medianSeries->setName(label);

            // …and force the legend to refresh
            auto markers = chart->legend()->markers(medianSeries);
            if (!markers.isEmpty()) {
                markers.first()->setLabel(label);
            }
            for (QLegendMarker* marker : chart->legend()->markers()) {
                marker->setLabelBrush(QBrush(Qt::white));
            }

        }
    }

    // 2) Reposition the vertical line
    if (verticalLine) {
        chart->scene()->removeItem(verticalLine);
        delete verticalLine;
        verticalLine = nullptr;
    }
    double minY = axisY->min();
    double maxY = axisY->max();
    QPointF top    = chart->mapToPosition(QPointF(currentMonth, maxY));
    QPointF bottom = chart->mapToPosition(QPointF(currentMonth, minY));
    verticalLine = new QGraphicsLineItem(QLineF(bottom, top));
    verticalLine->setPen(QPen(Qt::red, 2));
    chart->scene()->addItem(verticalLine);

    chart->update();
}




