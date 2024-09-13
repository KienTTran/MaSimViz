#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QList>
#include <vector>
#include <functional> // for std::function

#include "vizdata.h"

class DataProcessor {
public:
    // Constructor (optional)
    DataProcessor() {}

public:
    void processStatsData(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback);
};

#endif // DATAPROCESSOR_H
