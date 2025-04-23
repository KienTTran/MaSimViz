#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QList>
#include <vector>

#include <QObject>
#include "vizdata.h"

class DataProcessor : public QObject{
    Q_OBJECT
public:
    // Constructor (optional)
    DataProcessor() {}

public:
    void processStatsData(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback);
    void saveStatsDataToCSV(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback);
    void loadStatsDataFromCSV(const QString& tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void(int)> completionCallback);

    void saveAllValuesSummaryToCSV(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback);
    void loadAllValuesSummaryFromCSV(VizData* vizData, std::function<void(int)> progressCallback, std::function<void(int)> completionCallback);

    QList<VizData::GenotypeFrequency> readGenotypeFrequencyFromDatabase(const QString& dbPath);

    void saveGenotypeFrequenciesMatrixToCSV(const QList<VizData::GenotypeFrequency>& data,
                                            int totalMonths,
                                            int totalLocations,
                                            const QString& filePath);
    void loadGenotypeFrequenciesMatrixFromCSV(VizData* vizData, const QString& filePath);
    void computeGenotypeFrequencyRange(VizData* vizData);
    void sortGenotypeNames(VizData* vizData);
};

#endif // DATAPROCESSOR_H
