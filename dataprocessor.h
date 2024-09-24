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
};

#endif // DATAPROCESSOR_H
