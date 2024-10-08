
#include <QRandomGenerator>
#include <QAreaSeries>
#include <QLegendMarker>
#include <QCategoryAxis>
#include <QTimer>
#include "chartcustom.h"

ChartCustom::ChartCustom(QObject *parent)
    : QObject{parent}
{
    verticalLine = nullptr;
}

void ChartCustom::setVizData(VizData *vizData){
    this->vizData = vizData;
}

void ChartCustom::setChartView(QChartView *chartView){
    this->chartView = chartView;
    // this->chartView->setRubberBand(QChartView::HorizontalRubberBand);
}

void ChartCustom::plotDataMedianMultipleLocations(QString colName, QMap<QPair<int,int>,QColor> locInfo, int currentMonth, QString title) {
    // Check if the median data is available
    if (vizData->statsData[colName].iqr[0].isEmpty()) {
        return;
    }

    // Create a new chart object
    chart = new QChart();

    // Apply chart theme
    chart->setTheme(QChart::ChartThemeDark);

    chart->setTitle(QString("%1").arg(title));

    // Create a QValueAxis for the X axis (month) and Y axis (median values)
    QCategoryAxis *axisX = new QCategoryAxis;
    axisX->setRange(0, vizData->statsData[colName].iqr[0].size() - 1);  // Set the range for the months

    for (int i = 0; i < vizData->statsData[colName].iqr[0].size(); i += 12) {
        if(i == 0){
            axisX->append(QString("Year %1").arg(i / 12), i);
        }
        else if(i % 60 ==0){
            axisX->append(QString("%1").arg(i / 12), i);
        }
        else{
            axisX->append(QString(""), i);
        }
    }

    // Customize tick marks (optional)
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    // Add axisX to the chart
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%.2f");

    // Add axes to the chart
    chart->addAxis(axisY, Qt::AlignLeft);

    // chart->legend()->hide();

    // Initialize variables to track min and max Y-values
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();

    int locIndex = 0;
    // Create a different QLineSeries for each location and add it to the chart
    for (QPair<int,int> colrow : locInfo.keys()) {
        // Extract the location index and color
        QColor color = locInfo[colrow];

        if(vizData->isDistrictReporter){
            locIndex = vizData->rasterData->locationPair2DTo1DDistrict[colrow];
        }
        else{
            locIndex = vizData->rasterData->locationPair2DTo1D[colrow];
        }

        // Create a QLineSeries object for the current location's median line
        QLineSeries* medianSeries = new QLineSeries();
        QPen medianPen(color, 3); // Make the median line thicker and brighter
        medianPen.setColor(color.lighter(150)); // Make the color brighter
        medianSeries->setPen(medianPen);

        // Create two QLineSeries for IQR (iqr25 and iqr75)
        QLineSeries* iqr25Series = new QLineSeries();
        QLineSeries* iqr75Series = new QLineSeries();

        // Populate the series with the median, iqr25, and iqr75 data
        for (int month = 0; month < vizData->statsData[colName].iqr[0].size(); month++) {
            qreal medianValue = vizData->statsData[colName].iqr[0][month][locIndex];
            qreal iqr25Value = vizData->statsData[colName].iqr[2][month][locIndex];
            qreal iqr75Value = vizData->statsData[colName].iqr[3][month][locIndex];

            // Append the values to their respective series
            medianSeries->append(month, medianValue);
            iqr25Series->append(month, iqr25Value);
            iqr75Series->append(month, iqr75Value);

            // Update the min and max Y-values
            if (iqr25Value < minY) {
                minY = iqr25Value;
            }
            if (iqr75Value > maxY) {
                maxY = iqr75Value;
            }
        }

        // Create QAreaSeries to represent the area between iqr25 and iqr75
        QAreaSeries* iqrAreaSeries = new QAreaSeries(iqr25Series, iqr75Series);
        QBrush areaBrush(color);
        areaBrush.setStyle(Qt::SolidPattern);
        color.setAlphaF(0.4); // Set transparency to 20%
        areaBrush.setColor(color);
        iqrAreaSeries->setBrush(areaBrush);
        iqrAreaSeries->setPen(QPen(Qt::NoPen)); // No border for the area

        // Add the area series and median series to the chart
        chart->addSeries(iqrAreaSeries);
        chart->addSeries(medianSeries);

        // Attach axes to the series
        iqrAreaSeries->attachAxis(axisX);
        iqrAreaSeries->attachAxis(axisY);
        medianSeries->attachAxis(axisX);
        medianSeries->attachAxis(axisY);

        // Hide legend markers for iqrAreaSeries and medianSeries
        for (QLegendMarker* marker : chart->legend()->markers(iqrAreaSeries)) {
            marker->setVisible(false);  // Hide the area series legend marker
        }

        medianSeries->setName(QString("%1(%2 - %3)").arg(QString::number(vizData->statsData[colName].iqr[0][currentMonth][locIndex],'f',2),
                                                         QString::number(vizData->statsData[colName].iqr[2][currentMonth][locIndex],'f',2),
                                                         QString::number(vizData->statsData[colName].iqr[3][currentMonth][locIndex],'f',2)));
    }

    // Set the Y-axis range to include all data points
    axisY->setRange(minY, maxY);

    // Set the chart to the QChartView
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(chart);

    if(locInfo.size() > 0){
        // Ensure the current month is within the valid range
        if (currentMonth < 0 || currentMonth >= vizData->statsData[colName].iqr[0].size()) {
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
    }
    else{
        // Remove the previous vertical line if it exists
        if (verticalLine) {
            chart->scene()->removeItem(verticalLine);
            delete verticalLine;  // Clean up memory
            verticalLine = nullptr;
        }
    }

    // Add the vertical line to the chart's scene
    chart->scene()->addItem(verticalLine);
    chart->update();
}







