#include "dataprocessor.h"
#include <algorithm> // For std::nth_element and std::sort
#include <QtAlgorithms> // For qSort
#include <QtGlobal> // For qMin, qMax
#include <QVector>
#include <QtConcurrent> // For QtConcurrent and QFuture
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

// Function to process one StatsData object
void processSingleStatsData(VizData* vizData, VizData::StatsData& stats, int nDatabases, int nLocations, int nMonths) {
    // Initialize the dimensions for median, IQR, min, and max
    stats.median.resize(nMonths);
    stats.iqr5.resize(nMonths);
    stats.iqr25.resize(nMonths);
    stats.iqr75.resize(nMonths);
    stats.iqr95.resize(nMonths);

    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();
    double globalMedianMin = std::numeric_limits<double>::max();
    double globalMedianMax = std::numeric_limits<double>::lowest();

    // Process each month and each location
    for (int month = 0; month < nMonths; ++month) {
        stats.median[month].resize(nLocations);
        stats.iqr5[month].resize(nLocations);
        stats.iqr25[month].resize(nLocations);
        stats.iqr75[month].resize(nLocations);
        stats.iqr95[month].resize(nLocations);

        for (int loc = 0; loc < nLocations; ++loc) {
            QList<double> values;
            for (int db = 0; db < nDatabases; ++db) {
                values.append(stats.data[db][loc][month]);
            }

            // Calculate median and IQR
            stats.median[month][loc] = percentile(values, 50);
            stats.iqr5[month][loc] = percentile(values, 5);
            stats.iqr25[month][loc] = percentile(values, 25);
            stats.iqr75[month][loc] = percentile(values, 75);
            stats.iqr95[month][loc] = percentile(values, 95);

            // Update global min/max values from data
            double localMin = *std::min_element(values.begin(), values.end());
            double localMax = *std::max_element(values.begin(), values.end());
            globalMin = qMin(globalMin, localMin);
            globalMax = qMax(globalMax, localMax);

            // Update global min/max values for median
            globalMedianMin = qMin(globalMedianMin, stats.median[month][loc]);
            globalMedianMax = qMax(globalMedianMax, stats.median[month][loc]);
        }
    }

    // Assign global min/max values to stats
    stats.dataMin = globalMin;
    stats.dataMax = globalMax;
    stats.medianMin = globalMedianMin;
    stats.medianMax = globalMedianMax;
}


void DataProcessor::processStatsData(VizData* vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    int nDatabases = vizData->statsData[0].data.size();      // Assumed number of databases
    int nLocations = vizData->rasterData->locationRaster;    // Number of locations
    int nMonths = vizData->monthCountStartToEnd;             // Number of months
    int totalStats = vizData->statsData.size();              // Total number of statsData items to process

    // Create a QVector of QFuture and a QVector of QFutureWatcher
    QVector<QFuture<void>> futures;

    // Start a timer to periodically check progress
    QElapsedTimer timer;
    timer.start(); // Timer for controlling polling intervals

    // Iterate over each statData and process them in parallel
    for (int s = 0; s < totalStats; ++s) {
        VizData::StatsData& stats = vizData->statsData[s];

        // Launch each stats processing in a separate thread using QtConcurrent::run
        QFuture<void> future = QtConcurrent::run([=, &stats] {
            processSingleStatsData(vizData, stats, nDatabases, nLocations, nMonths);
        });

        futures.append(future);  // Add the future to the list
    }

    int totalProgress = 0;

    // Polling loop to check the progress and invoke the progress callback
    while (totalProgress < 100) {
        int completedFutures = 0;

        // Only poll futures if 100ms have passed since the last poll
        if (timer.elapsed() > 100) {
            // Check each future's progress
            for (int i = 0; i < futures.size(); ++i) {
                if (futures[i].isFinished()) {
                    completedFutures++;
                }
            }

            // Calculate and report progress
            int progress = (completedFutures * 100) / totalStats;
            if (progress > totalProgress) {
                totalProgress = progress;
                progressCallback(progress);  // Invoke the progress callback
            }

            // Reset the timer for the next poll
            timer.restart();
        }

        // If all futures are done, break the loop
        if (completedFutures == totalStats) {
            break;
        }

        // Allow other events to process (simulating idle polling)
        QCoreApplication::processEvents();
    }

    // All futures are completed, invoke the completion callback
    completionCallback();
}
