
#include <QRandomGenerator>

#include "chartcustom.h"

ChartCustom::ChartCustom(QObject *parent)
    : QObject{parent}
{
    verticalLine = nullptr;
}

#include <QTimer>

void ChartCustom::plotDataMedianMultipleLocations(QChartView* chartView, VizData *vizData, int colIndex, QMap<int,QColor> locInfo, int currentMonth, QString title) {
    // Check if the median data is available
    if (vizData->statsData[colIndex].median.isEmpty()) {
        return;
    }

    // Create a new chart object
    chart = new QChart();
    chart->setTitle(title + " (Multiple Locations)");

    // Create a QValueAxis for the X axis (month) and Y axis (median values)
    QValueAxis *axisX = new QValueAxis;
    axisX->setTickCount(20);

    QValueAxis *axisY = new QValueAxis;

    // Add axes to the chart
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // chart->legend()->hide();

    // Initialize variables to track min and max Y-values
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();

    // Create a different QLineSeries for each location and add it to the chart
    for (int locIndex : locInfo.keys()) {
        // Extract the location index and color
        QColor color = locInfo[locIndex];

        QPair<int, int> colrow = vizData->rasterData->locationPair1DTo2D[locIndex];

        // Create a QLineSeries object for the current location
        QLineSeries* series = new QLineSeries();
        series->setName(QString("%1").arg(vizData->statsData[colIndex].median[currentMonth][locIndex]));

        // Populate the series with the median data
        for (int month = 0; month < vizData->statsData[colIndex].median.size(); month++) {
            qreal yValue = vizData->statsData[colIndex].median[month][locIndex];
            series->append(month, yValue);

            // Update the min and max Y-values
            if (yValue < minY) {
                minY = yValue;
            }
            if (yValue > maxY) {
                maxY = yValue;
            }
        }

        series->setColor(color);

        qDebug() << "[Chart]Location index: " << locIndex << "Color: " << color << "linecolor" << series->color();

        // Add the series to the chart
        chart->addSeries(series);

        // Attach axes to the series
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
    qDebug() << "\n";

    // Set the Y-axis range to include all data points
    axisY->setRange(minY, maxY);

    // Apply chart theme
    chart->setTheme(QChart::ChartThemeDark);

    // Set the chart to the QChartView
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(chart);

    // Ensure the current month is within the valid range
    if (currentMonth < 0 || currentMonth >= vizData->statsData[colIndex].median.size()) {
        return;
    }

    // Remove the previous vertical line if it exists
    if (verticalLine) {
        chart->scene()->removeItem(verticalLine);
        delete verticalLine;  // Clean up memory
        verticalLine = nullptr;
    }

    // Calculate the position of the vertical line
    QPointF topPosition = chart->mapToPosition(QPointF(currentMonth, maxY));
    QPointF bottomPosition = chart->mapToPosition(QPointF(currentMonth, minY));

    // Check if the positions are valid before proceeding
    if (topPosition == QPointF(0, 0) || bottomPosition == QPointF(0, 0)) {
        qDebug() << "Invalid positions for vertical line: topPosition:" << topPosition << "bottomPosition:" << bottomPosition;
        return;
    }

    // Create a new QGraphicsLineItem for the vertical line
    verticalLine = new QGraphicsLineItem(QLineF(bottomPosition, topPosition));
    QPen vlinePen(Qt::red);
    vlinePen.setWidth(2);  // Set line width
    verticalLine->setPen(vlinePen);

    // Add the vertical line to the chart's scene
    chart->scene()->addItem(verticalLine);
    chart->update();
}





