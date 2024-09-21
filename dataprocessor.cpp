#include "dataprocessor.h"
#include <QtConcurrent> // For QtConcurrent::run
#include <QFutureWatcher>
#include <QVector>
#include <algorithm>
#include <limits>
#include <QtAlgorithms>
#include <QtGlobal>
#include <functional> // For std::function
#include <QDebug> // For logging

#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QList>
#include <QStringList>

#include "vizdata.h"

// Helper function to compute percentiles
double percentile(QList<double> &values, double percent) {
    int n = values.size();
    if (n == 0) return 0;
    std::sort(values.begin(), values.end());
    double k = (n - 1) * (percent / 100.0);
    int f = qFloor(k);
    int c = qCeil(k);
    if (f == c) {
        return values[f];
    }
    return values[f] + (k - f) * (values[c] - values[f]);
}

// Parallel function to calculate percentiles for a given stat and store it
void calculatePercentiles(VizData::StatsData& stats, QList<double> values, int month, int loc, double percent, QVector<QVector<double>>& outputVector) {
    outputVector[month][loc] = percentile(values, percent);  // Assign to double
}

// Helper function to update global min/max values
void updateGlobalMinMax(double localValue, double& globalMin, double& globalMax) {
    globalMin = qMin(globalMin, localValue);
    globalMax = qMax(globalMax, localValue);
}

// Actual work function to run in a separate thread for all IQRs/medians
void processStatsDataWorker(VizData* vizData, std::function<void(int)> progressCallback) {
    int nDatabases = vizData->statsData[vizData->statsData.keys()[0]].data.size();  // Number of databases
    int nLocations = vizData->rasterData->nLocations; // Number of locations
    int nMonths = vizData->monthCountStartToEnd;  // Number of months
    int progress = 0;

    QVector<QFuture<void>> futures; // Store futures to wait for all threads to finish

    // Iterate over each statData
    for (int colNameIndex = 0; colNameIndex < vizData->statsData.keys().size(); colNameIndex++) {
        VizData::StatsData& stats = vizData->statsData[vizData->statsData.keys()[colNameIndex]];

        // Initialize the dimensions for median, IQR, min, and max
        stats.iqr = QList<QList<QList<double>>>(stats.iqrRanges.size(), QList<QList<double>>(nMonths, QList<double>(nLocations, 0.0)));

        double globalMin = std::numeric_limits<double>::max();
        double globalMax = std::numeric_limits<double>::lowest();
        double globalMedianMin = std::numeric_limits<double>::max();
        double globalMedianMax = std::numeric_limits<double>::lowest();

        // Process each month and each location
        for (int month = 0; month < nMonths; ++month) {
            for (int loc = 0; loc < nLocations; ++loc) {
                QList<double> values;
                for (int db = 0; db < nDatabases; ++db) {
                    values.append(stats.data[db][loc][month]);
                }

                if (progressCallback) {
                    progressCallback(1);  // Report percentage progress, normalized to 0-100
                }
                for(int i = 0; i < stats.iqrRanges.size(); i++){
                    // futures.append(QtConcurrent::run(calculatePercentiles, std::ref(stats), values, month, loc, stats.iqrRanges[i], std::ref(stats.iqr[i])));
                    calculatePercentiles(stats, values, month, loc, stats.iqrRanges[i], stats.iqr[i]);
                }

                // Update global min/max values from data
                double localMin = *std::min_element(values.begin(), values.end());
                double localMax = *std::max_element(values.begin(), values.end());
                updateGlobalMinMax(localMin, globalMin, globalMax);
            }
        }

        // Wait for all threads to complete
        for (auto& future : futures) {
            future.waitForFinished();
        }

        // After all percentile calculations are done, update the global median min/max values
        for (int month = 0; month < nMonths; ++month) {
            for (int loc = 0; loc < nLocations; ++loc) {
                updateGlobalMinMax(stats.iqr[0][month][loc], globalMedianMin, globalMedianMax);
            }
        }

        // Assign global min/max values to stats
        stats.dataMin = globalMin;
        stats.dataMax = globalMax;
        stats.medianMin = globalMedianMin;
        stats.medianMax = globalMedianMax;

        if (progressCallback) {
            progressCallback((colNameIndex * 100) / vizData->statsData.keys().size());  // Report percentage progress, normalized to 0-100
        }
    }
}


// Function to process data and fill StatsData asynchronously using QFutureWatcher
void DataProcessor::processStatsData(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Use QFuture and QFutureWatcher to run the function in a separate thread
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>();

    // Connect to signals to track progress and completion
    QObject::connect(watcher, &QFutureWatcher<void>::finished, [=]() {
        if (completionCallback) {
            completionCallback();
        }
        watcher->deleteLater();  // Clean up the watcher
    });

    // Run the actual work in a separate thread
    QFuture<void> future = QtConcurrent::run(processStatsDataWorker, vizData, progressCallback);

    // Set the future to the watcher so it can monitor the progress
    watcher->setFuture(future);
}

void saveToCSVWorker(VizData *vizData,std::function<void(int)> progressCallback) {
    QString fileName = QDir(vizData->currentDirectory).filePath("MaSimViz_"+vizData->sqlData.tableColumnsMap.keys().last() + ".dat");
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Unable to open file for writing.");
        return;
    }

    QTextStream out(&file);

    QString writeString = "";

    qDebug() << "[Save]Selected columns:" << vizData->sqlData.tableColumnsMap.values().last();
    qDebug() << "[Save]statsData columns:" << vizData->statsData.keys();
    qDebug() << "[Save]raster nLocation:" << vizData->rasterData->nLocations;

    //sort the columns based on the column name
    QStringList colNamesSorted = vizData->statsData.keys();
    colNamesSorted.sort();

    writeString = "";
    for (const QString col : colNamesSorted) {
        writeString += col + ",";
    }
    writeString.chop(1);
    out << writeString << "\n";

    qDebug() << "[Save]Columns:" << writeString;

    writeString = "";
    for (const QString col : colNamesSorted){
        writeString += QString::number(vizData->statsData[col].medianMin) + ",";
    }
    writeString.chop(1);
    out << writeString << "\n";

    qDebug() << "[Save]Stats min:" << writeString;

    writeString = "";
    for (const QString col : colNamesSorted){
        writeString += QString::number(vizData->statsData[col].medianMax) + ",";
    }
    writeString.chop(1);
    out << writeString << "\n";

    qDebug() << "[Save]Stats max:" << writeString;

    // Prepare header
    writeString = "";
    for (const QString col : colNamesSorted) {
        for (int loc = 0; loc < vizData->rasterData->nLocations; loc++) {
            writeString += QString("%1_%2_median").arg(col).arg(loc) + ",";
            writeString += QString("%1_%2_iqr25").arg(col).arg(loc) + ",";
            writeString += QString("%1_%2_iqr75").arg(col).arg(loc) + ",";
            writeString += QString("%1_%2_iqr5").arg(col).arg(loc) + ",";
            writeString += QString("%1_%2_iqr95").arg(col).arg(loc) + ",";
        }
    }
    writeString.chop(1);
    out << writeString << "\n";

    writeString = "";
    // Write data
    for (int month = 0; month < vizData->monthCountStartToEnd; month++) {
        writeString = "";
        int count = 0;
        for (int colNameIndex = 0; colNameIndex < colNamesSorted.size(); colNameIndex++) {
            for (int loc = 0; loc < vizData->rasterData->nLocations; loc++) {
                for(int i = 0; i < 5; i++){
                    int index = colNameIndex * vizData->rasterData->nLocations * 5 + loc * 5 + i;
                    writeString += QString::number(vizData->statsData[colNamesSorted[colNameIndex]].iqr[i][month][loc]) +",";
                    if((month == 0 || month == 1) && i == 0){
                        if(count == vizData->rasterData->nLocations-1){
                            qDebug() << "[Save]Last:" << count << month << colNamesSorted[colNameIndex] << loc << index << vizData->statsData[colNamesSorted[colNameIndex]].iqr[4][month][loc];
                        }
                        if(count == vizData->rasterData->nLocations*2-1){
                            qDebug() << "[Save]Last:" << count << month << colNamesSorted[colNameIndex] << loc << index << vizData->statsData[colNamesSorted[colNameIndex]].iqr[4][month][loc];
                        }
                        if(count == vizData->rasterData->nLocations*3-1){
                            qDebug() << "[Save]Last:" << count << month << colNamesSorted[colNameIndex] << loc << index << vizData->statsData[colNamesSorted[colNameIndex]].iqr[4][month][loc];
                        }
                    }
                }
                count++;
            }
        }
        // qDebug() << "[Save]Writing month:" << month << "count:" << count;
        writeString.chop(1);

        // QStringList test = writeString.split(",");
        // qDebug() << "[Save]month:" << month << count << "writeString size:" << test.size();

        out << writeString << "\n";
        if(progressCallback)
            progressCallback((month*100)/vizData->monthCountStartToEnd);
    }

    file.close();
}

// Function to save VizData to CSV
void DataProcessor::saveStatsDataToCSV(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Use QFuture and QFutureWatcher to run the function in a separate thread
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>();

    // Connect to signals to track progress and completion
    QObject::connect(watcher, &QFutureWatcher<void>::finished, [=]() {
        if (completionCallback) {
            completionCallback();
        }
        watcher->deleteLater();  // Clean up the watcher
    });

    // Run the actual work in a separate thread
    QFuture<void> future = QtConcurrent::run(saveToCSVWorker, vizData, progressCallback);

    // Set the future to the watcher so it can monitor the progress
    watcher->setFuture(future);
}

// Function to read VizData from CSV
int readFromCSVWorker(const QString& tableName, VizData *vizData,std::function<void(int)> progressCallback) {
    QString fileName = QDir(vizData->currentDirectory).filePath("MaSimViz_"+vizData->sqlData.tableColumnsMap.keys().last() + ".dat");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text) | !file.exists()) {
        qWarning("Unable to open file for reading or file does not exist.");
        return 1;
    }

    QTextStream in(&file);
    QStringList colNames = in.readLine().split(",");
    QStringList statsMin = in.readLine().split(",");
    QStringList statsMax = in.readLine().split(",");
    QStringList header = in.readLine().split(",");

    colNames.sort();

    qDebug() << "[Load]Selected columns:" << vizData->sqlData.tableColumnsMap.values();
    qDebug() << "[Load]Reading data from CSV file:" << fileName;
    qDebug() << "[Load]Column names:" << colNames;
    qDebug() << "[Load]Stats min:" << statsMin;
    qDebug() << "[Load]Stats max:" << statsMax;

    vizData->sqlData.tableColumnsMap[tableName] = colNames.join(",").chopped(1);

    if(colNames.size() != vizData->statsData.size()){
        qWarning("Column number does not match.");
        return 2;
    }

    if(colNames.size()*vizData->rasterData->nLocations*5 != header.size()){
        qWarning("Header size does not match config location and columns.");
        return 3;
    }

    bool allMatch = true;
    for(const QString col: colNames){
        if(!vizData->statsData.contains(col)){
            allMatch = false;
            qWarning("Column names do not match selected columns.");
            return 4;
        }
    }


    // Clear the existing data
    for(const QString col : colNames){
        vizData->statsData[col].medianMin = statsMin[colNames.indexOf(col)].toDouble();
        vizData->statsData[col].medianMax = statsMax[colNames.indexOf(col)].toDouble();
        vizData->statsData[col].iqr = QList<QList<QList<double>>>(vizData->statsData[col].iqrRanges.size(), QList<QList<double>>(vizData->monthCountStartToEnd, QList<double>(vizData->rasterData->nLocations, 0.0)));
    }

    int month = 0;
    while(!in.atEnd()){
        QString readLine = in.readLine();
        QStringList line = readLine.split(",");
        // qDebug() << "[Load]month:" << month << "line length:" << line.size();
        int count = 0;
        for(int colNameIndex = 0; colNameIndex < colNames.size(); colNameIndex++){
            for (int loc = 0; loc < vizData->rasterData->nLocations; loc++) {
                int iqrSize =  vizData->statsData[colNames[colNameIndex]].iqrRanges.size();
                for(int i = 0; i < iqrSize; i++){
                    int index = colNameIndex * vizData->rasterData->nLocations *  iqrSize + loc *  iqrSize + i;
                    // qDebug() << "month:" << month << colNames[colNameIndex] << "loc:" << loc << "index:" << index;
                    vizData->statsData[colNames[colNameIndex]].iqr[i][month][loc] = line[index].toDouble();
                    if((month == 0 || month == 1) && i == 0){
                        if(count == vizData->rasterData->nLocations-1){
                            qDebug() << "[Load]Last:" << count << month << colNames[colNameIndex] << loc << index << line[index].toDouble();
                        }
                        if(count == vizData->rasterData->nLocations*2-1){
                            qDebug() << "[Load]Last:" << count << month << colNames[colNameIndex] << loc << index << line[index].toDouble();
                        }
                        if(count == vizData->rasterData->nLocations*3-1){
                            qDebug() << "[Load]Last:" << count << month << colNames[colNameIndex] << loc << index << line[index].toDouble();
                        }
                    }
                }
                count++;
            }
        }
        // qDebug() << "[Load]Reading month:" << month << "count:" << count;
        if(progressCallback)
            progressCallback((month*100)/vizData->monthCountStartToEnd);
        month++;
    }

    file.close();
    return 0;
}

void DataProcessor::loadStatsDataFromCSV(const QString& tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void(int)> completionCallback){
    // Use QFuture and QFutureWatcher to run the function in a separate thread
    QFutureWatcher<int> *watcher = new QFutureWatcher<int>();

    // Run the actual work in a separate thread
    QFuture<int> future = QtConcurrent::run(readFromCSVWorker, tableName, vizData, progressCallback);

    // Set the future to the watcher so it can monitor the progress
    watcher->setFuture(future);


    // Connect to signals to track progress and completion
    QObject::connect(watcher, &QFutureWatcher<void>::finished, [=]() {
        if (completionCallback) {
            completionCallback(future.result());
        }
        watcher->deleteLater();  // Clean up the watcher
    });
}


