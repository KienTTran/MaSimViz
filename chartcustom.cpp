#include "chartcustom.h"

ChartCustom::ChartCustom(QObject *parent)
    : QObject{parent}
{}


void ChartCustom::plotVerticalLineOnChart(VizData *vizData, int colIndex, int locIndex, int month) {
    if (!chart || !series) {
        emit valueAtVerticalLineChanged(0.0);
        return;  // Ensure chart and series are initialized before proceeding
    }

    // If a vertical line already exists, remove it from the chart
    if (currentVerticalLine) {
        chart->removeSeries(currentVerticalLine);
        delete currentVerticalLine;  // Clean up memory
        currentVerticalLine = nullptr;
    }

    // If a label exists, remove it from the scene
    if (valueLabel) {
        chart->scene()->removeItem(valueLabel);
        delete valueLabel;  // Clean up memory
        valueLabel = nullptr;
    }

    // Get the current range of the Y-axis
    QValueAxis *axisY = qobject_cast<QValueAxis *>(chart->axisY());
    if (!axisY) {
        emit valueAtVerticalLineChanged(0.0);
        return;
    }

    // Get the min and max values of the Y-axis
    qreal minY = axisY->min();
    qreal maxY = axisY->max();

    // Create a new QLineSeries for the vertical line
    currentVerticalLine = new QLineSeries();

    // Add two points to create a vertical line at the specified month
    currentVerticalLine->append(month, minY);  // Point at the bottom
    currentVerticalLine->append(month, maxY);  // Point at the top

    // Set the vertical line's appearance (e.g., dashed line, color)
    QPen linePen(Qt::SolidLine);  // Use a dashed line
    linePen.setColor(Qt::red);   // Set color to red, for example
    linePen.setWidth(2);         // Set line width
    currentVerticalLine->setPen(linePen);

    // Add the vertical line series to the chart
    chart->addSeries(currentVerticalLine);

    // Attach the axes to the vertical line
    chart->setAxisX(chart->axisX(), currentVerticalLine);
    chart->setAxisY(chart->axisY(), currentVerticalLine);

    // Now display the Y value at the point where the vertical line crosses the data series
    qreal yValue = vizData->statsData[colIndex].median[month][locIndex];  // Get the Y-value at the specified month

    // Create a label to display the Y-value
    valueLabel = new QGraphicsSimpleTextItem(QString::number(yValue));

    // Find the position of the label on the chart (convert chart coordinates to scene coordinates)
    QPointF labelPos = chart->mapToPosition(QPointF(month, yValue), currentVerticalLine);

    // Set the position of the label slightly above the intersection point
    valueLabel->setPos(labelPos.x(), labelPos.y() - 20);  // Offset slightly upwards to avoid overlapping with the point

    valueLabel->setPen(QPen(Qt::white));
    valueLabel->setBrush(QBrush(Qt::white));

    // Add the label to the chart's scene
    chart->scene()->addItem(valueLabel);

    emit valueAtVerticalLineChanged(yValue);

}

#include <QGraphicsTextItem>

void ChartCustom::plotDataMedian1Location(QChartView* chartView, VizData *vizData, int colIndex, int locIndex, QString title)
{
    // Plot graphs using QCharts
    if (vizData->statsData[colIndex].median.isEmpty()) {
        return;
    }

    QPair<int, int> colrow = vizData->rasterData->locationPair1DTo2D[locIndex];
    // Create a QChart object
    chart = new QChart();
    chart->setTitle(title + " location:" + QString::number(locIndex) + " (row: " + QString::number(colrow.first) + ", col: " + QString::number(colrow.second) + ")");

    // Create a QLineSeries object for the data
    series = new QLineSeries();

    // Populate the series with the median data
    for (int month = 0; month < vizData->statsData[colIndex].median.size(); month++) {
        series->append(month, vizData->statsData[colIndex].median[month][locIndex]);
    }

    // Add the series to the chart
    chart->addSeries(series);
    chart->legend()->hide();  // Hide the legend

    // Create axes
    axisX = new QValueAxis;
    axisX->setTitleText("Month");
    axisX->setTickCount(20);

    axisY = new QValueAxis;

    // Add the axes to the chart
    chart->setAxisX(axisX, series);
    chart->setAxisY(axisY, series);

    chart->setTheme(QChart::ChartThemeDark);

    // Create a QChartView to display the chart
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(chart);
}

