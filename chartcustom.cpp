
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

    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(chart);

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

void ChartCustom::plotSummaryDataOnly(const QString& colName, int currentMonth, const QString& title) {
    if (!vizData->statsDataSummary.contains(colName) || vizData->statsDataSummary[colName].median.isEmpty()) {
        qWarning() << "[ChartCustom] Summary stats not found for column:" << colName;
        return;
    }

    const auto& summary = vizData->statsDataSummary[colName];
    int monthCount = summary.median.size();

    // Create chart
    chart = new QChart();
    chart->setTheme(QChart::ChartThemeDark);
    chart->setTitle(title);

    // X Axis (months)
    QCategoryAxis* axisX = new QCategoryAxis;
    axisX->setRange(0, monthCount - 1);
    for (int i = 0; i < monthCount; i += 12) {
        axisX->append((i == 0 || i % 60 == 0) ? QString("Year %1").arg(i / 12) : "", i);
    }
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Y Axis
    QValueAxis* axisY = new QValueAxis;
    axisY->setLabelFormat("%.2f");
    chart->addAxis(axisY, Qt::AlignRight);

    // Create summary line and area series
    QLineSeries* medianSeries = new QLineSeries();
    QLineSeries* iqr25Series = new QLineSeries();
    QLineSeries* iqr75Series = new QLineSeries();

    QVector<QPointF> medianPoints, iqr25Points, iqr75Points;
    medianPoints.reserve(monthCount);
    iqr25Points.reserve(monthCount);
    iqr75Points.reserve(monthCount);

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

    QAreaSeries* areaSeries = new QAreaSeries(iqr25Series, iqr75Series);
    QColor fillColor = Qt::cyan;
    fillColor.setAlphaF(0.4);
    areaSeries->setBrush(QBrush(fillColor));
    areaSeries->setPen(Qt::NoPen);

    QPen medianPen(Qt::cyan, 3);
    medianSeries->setPen(medianPen);
    medianSeries->setName(QString("Median: %1 (%2 - %3)")
                              .arg(QString::number(summary.median[currentMonth], 'f', 2))
                              .arg(QString::number(summary.iqr25[currentMonth], 'f', 2))
                              .arg(QString::number(summary.iqr75[currentMonth], 'f', 2)));

    chart->addSeries(areaSeries);
    chart->addSeries(medianSeries);

    areaSeries->attachAxis(axisX);
    areaSeries->attachAxis(axisY);
    medianSeries->attachAxis(axisX);
    medianSeries->attachAxis(axisY);

    for (QLegendMarker* marker : chart->legend()->markers(areaSeries)) {
        marker->setVisible(false);
    }

    axisY->setRange(minY, maxY);

    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(chart);

    // Vertical line for current month
    if (currentMonth >= 0 && currentMonth < monthCount) {
        QPointF top = chart->mapToPosition(QPointF(currentMonth, maxY));
        QPointF bottom = chart->mapToPosition(QPointF(currentMonth, minY));

        if (!verticalLine) {
            verticalLine = new QGraphicsLineItem();
            QPen vPen(Qt::red);
            vPen.setWidth(2);
            verticalLine->setPen(vPen);
            chart->scene()->addItem(verticalLine);
        }

        verticalLine->setLine(QLineF(bottom, top));
    }

    chart->update();
}


void ChartCustom::plotGenotypeFrequencyChart(QChartView* chartView,
                                             const QList<VizData::GenotypeFrequency>& data,
                                             double minFreq,
                                             double maxFreq) {
    QChart* chart = new QChart();
    chart->setTitle(QString("Genotype Frequency Over Time (%.2f - %.2f)").arg(minFreq).arg(maxFreq));
    chart->setTheme(QChart::ChartThemeDark);

    // Organize: aa_sequence → month → avg frequency
    QMap<QString, QMap<int, QList<double>>> freqMap;

    for (const auto& row : data) {
        freqMap[row.aa_sequence][row.monthlydataid].append(row.frequency);
    }

    QValueAxis* axisX = new QValueAxis;
    axisX->setTitleText("Month");
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis* axisY = new QValueAxis;
    axisY->setTitleText("Genotype Frequency");
    axisY->setLabelFormat("%.4f");
    chart->addAxis(axisY, Qt::AlignLeft);

    int colorIdx = 0;
    QList<QColor> palette = {Qt::red, Qt::blue, Qt::green, Qt::yellow, Qt::cyan, Qt::magenta, Qt::gray, Qt::darkGreen};

    for (const auto& aa_seq : freqMap.keys()) {
        const auto& monthFreqs = freqMap[aa_seq];

        // Compute average across all months for filtering
        double total = 0.0;
        int count = 0;
        for (const auto& freqs : monthFreqs) {
            total += std::accumulate(freqs.begin(), freqs.end(), 0.0);
            count += freqs.size();
        }
        if (count == 0) continue;

        double avgAcrossTime = total / count;
        if (avgAcrossTime < minFreq || avgAcrossTime > maxFreq) continue;

        // Create series
        QLineSeries* series = new QLineSeries();
        series->setName(aa_seq);
        QPen pen(palette[colorIdx++ % palette.size()]);
        pen.setWidth(2);
        series->setPen(pen);

        for (auto it = monthFreqs.begin(); it != monthFreqs.end(); ++it) {
            double avg = std::accumulate(it.value().begin(), it.value().end(), 0.0) / it.value().size();
            series->append(it.key(), avg);
        }

        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
}






