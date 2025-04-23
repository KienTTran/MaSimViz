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
    long totalNumbers = vizData->statsData.keys().size() * nMonths * nLocations * vizData->statsData[vizData->statsData.keys()[0]].iqrRanges.size();
    long progress = 0;

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

                for(int i = 0; i < stats.iqrRanges.size(); i++){
                    // futures.append(QtConcurrent::run(calculatePercentiles, std::ref(stats), values, month, loc, stats.iqrRanges[i], std::ref(stats.iqr[i])));
                    calculatePercentiles(stats, values, month, loc, stats.iqrRanges[i], stats.iqr[i]);
                }

                int progressPercentage = (progress * 100) / (totalNumbers);
                // qDebug() << "[DataProcessor] Progress:" << progress << "/" << vizData->statsData.keys().size() * nMonths * nLocations * stats.iqrRanges.size();
                progress += stats.iqrRanges.size();

                if (progressCallback) {
                    progressCallback(progressPercentage);
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
        if(progressCallback){
            int percent = (month*100)/vizData->monthCountStartToEnd;
            if(percent % 5 == 0){
                progressCallback(percent);
            }
        }
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
    qDebug() << "[Load]Stats data columns:" << vizData->statsData.keys();
    qDebug() << "[Load]Reading data from CSV file:" << fileName;
    qDebug() << "[Load]Available column names:" << colNames;
    qDebug() << "[Load]Stats min:" << statsMin;
    qDebug() << "[Load]Stats max:" << statsMax;

    // vizData->sqlData.tableColumnsMap[tableName] = colNames.join(",").chopped(1);

    // if(colNames.size() != vizData->statsData.size()){
    //     qWarning("Column number does not match.");
    //     return 2;
    // }

    // if(colNames.size()*vizData->rasterData->nLocations*5 != header.size()){
    //     qWarning("Header size does not match config location and columns.");
    //     return 3;
    // }

    bool allMatch = true;
    for(const QString col: vizData->statsData.keys()){
        if(!colNames.contains(col)){
            allMatch = false;
            qWarning("Column names do not match selected columns.");
            return 4;
        }
    }


    // Clear the existing data
    for(const QString col : vizData->statsData.keys()){
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
            if(vizData->statsData.keys().contains(colNames[colNameIndex])){
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
        }
        // qDebug() << "[Load]Reading month:" << month << "count:" << count;
        if(progressCallback){
            int percent = (month*100)/vizData->monthCountStartToEnd;
            if(percent % 5 == 0){
                progressCallback(percent);
            }
        }
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

void saveAllValuesSummaryToCSVWorker(VizData *vizData, std::function<void(int)> progressCallback) {
    QString fileName = QDir(vizData->currentDirectory).filePath("MaSimViz_all_values_summary.dat");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Unable to open summary file for writing.");
        return;
    }

    QTextStream out(&file);
    QStringList colNamesSorted = vizData->statsData.keys();
    colNamesSorted.sort();

    // Header
    out << "Month";
    for (const QString& col : colNamesSorted) {
        out << "," << col + "_median";
        out << "," << col + "_iqr25";
        out << "," << col + "_iqr75";
        out << "," << col + "_iqr5";
        out << "," << col + "_iqr95";
    }
    out << "\n";


    // Clear the existing data
    for(const QString col : vizData->statsData.keys()){
        vizData->statsDataSummary[col].median.clear();
        vizData->statsDataSummary[col].iqr5.clear();
        vizData->statsDataSummary[col].iqr25.clear();
        vizData->statsDataSummary[col].iqr75.clear();
        vizData->statsDataSummary[col].iqr95.clear();
    }

    for (int month = 0; month < vizData->monthCountStartToEnd; ++month) {
        out << month;
        for (const QString& col : colNamesSorted) {
            QList<double> allValues;
            VizData::StatsData& stats = vizData->statsData[col];
            for (const auto& db : stats.data) {
                for (int loc = 0; loc < vizData->rasterData->nLocations; ++loc) {
                    allValues.append(db[loc][month]);
                }
            }

            double med = percentile(allValues, 50.0);
            double p25 = percentile(allValues, 25.0);
            double p75 = percentile(allValues, 75.0);
            double p5 = percentile(allValues, 5.0);
            double p95 = percentile(allValues, 95.0);

            vizData->statsDataSummary[col].median.append(med);
            vizData->statsDataSummary[col].iqr25.append(p25);
            vizData->statsDataSummary[col].iqr75.append(p75);
            vizData->statsDataSummary[col].iqr5.append(p5);
            vizData->statsDataSummary[col].iqr95.append(p95);

            out << "," << med << "," << p25 << "," << p75 << "," << p5 << "," << p95;
        }
        out << "\n";

        if (progressCallback && (month * 100 / vizData->monthCountStartToEnd) % 5 == 0) {
            progressCallback((month * 100) / vizData->monthCountStartToEnd);
        }
    }

    file.close();
}

void DataProcessor::saveAllValuesSummaryToCSV(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
    QObject::connect(watcher, &QFutureWatcher<void>::finished, [=]() {
        if (completionCallback) completionCallback();
        watcher->deleteLater();
    });

    QFuture<void> future = QtConcurrent::run(saveAllValuesSummaryToCSVWorker, vizData, progressCallback);
    watcher->setFuture(future);
}

int loadAllValuesSummaryFromCSVWorker(VizData* vizData, std::function<void(int)> progressCallback) {
    QString fileName = QDir(vizData->currentDirectory).filePath("MaSimViz_all_values_summary.dat");
    QFile file(fileName);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Unable to open summary file for reading.");
        return 1;
    }

    QTextStream in(&file);
    QStringList header = in.readLine().split(",");
    QStringList colNames;
    for (int i = 1; i < header.size(); i += 5) {
        colNames.append(header[i].split("_").first());
    }

    for (const QString& col : colNames) {
        vizData->statsDataSummary[col] = VizData::StatsDataSummary();
    }

    int month = 0;
    while (!in.atEnd()) {
        QStringList line = in.readLine().split(",");
        for (int colIndex = 0; colIndex < colNames.size(); ++colIndex) {
            int base = 1 + colIndex * 5;
            vizData->statsDataSummary[colNames[colIndex]].median.append(line[base].toDouble());
            vizData->statsDataSummary[colNames[colIndex]].iqr25.append(line[base + 1].toDouble());
            vizData->statsDataSummary[colNames[colIndex]].iqr75.append(line[base + 2].toDouble());
            vizData->statsDataSummary[colNames[colIndex]].iqr5.append(line[base + 3].toDouble());
            vizData->statsDataSummary[colNames[colIndex]].iqr95.append(line[base + 4].toDouble());
        }
        if (progressCallback && (month * 100 / vizData->monthCountStartToEnd) % 5 == 0)
            progressCallback((month * 100) / vizData->monthCountStartToEnd);
        ++month;
    }

    file.close();
    return 0;
}

void DataProcessor::loadAllValuesSummaryFromCSV(VizData* vizData, std::function<void(int)> progressCallback, std::function<void(int)> completionCallback) {
    QFutureWatcher<int> *watcher = new QFutureWatcher<int>();
    QFuture<int> future = QtConcurrent::run(loadAllValuesSummaryFromCSVWorker, vizData, progressCallback);

    QObject::connect(watcher, &QFutureWatcher<void>::finished, [=]() {
        if (completionCallback) completionCallback(future.result());
        watcher->deleteLater();
    });

    watcher->setFuture(future);
}



#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QMap>
#include <QDebug>

QList<VizData::GenotypeFrequency> DataProcessor::readGenotypeFrequencyFromDatabase(const QString& dbPath) {
    QList<VizData::GenotypeFrequency> data;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "genotype_freq_conn");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qWarning() << "Cannot open database:" << db.lastError().text();
        return data;
    }

    QString queryStr = R"(
        SELECT
            mgd.monthlydataid,
            mgd.locationid,
            g.name AS aa_sequence,
            mgd.weightedoccurrences,
            msd.infectedindividuals
        FROM monthlygenomedata mgd
        JOIN monthlysitedata msd
            ON mgd.monthlydataid = msd.monthlydataid
            AND mgd.locationid = msd.locationid
        JOIN genotype g ON mgd.genomeid = g.id;
    )";

    QSqlQuery query(db);
    if (!query.exec(queryStr)) {
        qWarning() << "Query failed:" << query.lastError().text();
        return data;
    }

    while (query.next()) {
        int month = query.value("monthlydataid").toInt();
        int loc = query.value("locationid").toInt();
        QString aa_seq = query.value("aa_sequence").toString();
        double weight = query.value("weightedoccurrences").toDouble();
        double infected = query.value("infectedindividuals").toDouble();
        double freq = (infected > 0.0) ? weight / infected : 0.0;

        // qDebug() << loc << month << aa_seq << freq;

        data.append({month, loc, aa_seq, freq});
    }

    db.close();
    QSqlDatabase::removeDatabase("genotype_freq_conn");
    return data;
}


#include <QFile>
#include <QTextStream>

void DataProcessor::saveGenotypeFrequenciesToCSV(const QList<VizData::GenotypeFrequency>& data, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot write file:" << file.errorString();
        return;
    }

    QTextStream out(&file);
    out << "monthlydataid,locationid,aa_sequence,frequency\n";

    for (const auto& row : data) {
        out << row.monthlydataid << "," << row.locationid << ","
            << row.aa_sequence << "," << row.frequency << "\n";
    }

    file.close();
}

QList<VizData::GenotypeFrequency> DataProcessor::loadGenotypeFrequenciesFromCSV(const QString& filePath) {
    QList<VizData::GenotypeFrequency> data;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot read file:" << file.errorString();
        return data;
    }

    QTextStream in(&file);
    QString header = in.readLine(); // skip header

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() != 4) continue;

        VizData::GenotypeFrequency gf;
        gf.monthlydataid = parts[0].toInt();
        gf.locationid = parts[1].toInt();
        gf.aa_sequence = parts[2];
        gf.frequency = parts[3].toDouble();
        data.append(gf);
    }

    file.close();
    return data;
}

void DataProcessor::computeGenotypeFrequencyRange(VizData* vizData) {
    vizData->genotypeFrequencyRange.clear();

    QMap<QString, QList<double>> sequenceToValues;
    for (const auto& row : vizData->genotypeFrequencies) {
        sequenceToValues[row.aa_sequence].append(row.frequency);
    }

    for (auto it = sequenceToValues.begin(); it != sequenceToValues.end(); ++it) {
        const QList<double>& values = it.value();
        if (!values.isEmpty()) {
            double minVal = *std::min_element(values.begin(), values.end());
            double maxVal = *std::max_element(values.begin(), values.end());
            vizData->genotypeFrequencyRange[it.key()] = qMakePair(minVal, maxVal);
        }
    }
}


