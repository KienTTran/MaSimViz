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

// Actual work function to run in a separate thread
void processStatsDataWorker(VizData* vizData, std::function<void(int)> progressCallback) {
    int nDatabases = vizData->statsData[0].data.size();  // Number of databases
    int nLocations = vizData->rasterData->locationRaster; // Number of locations
    int nMonths = vizData->monthCountStartToEnd;  // Number of months
    int totalIterations = nMonths * nLocations * 5 * vizData->statsData.size(); // Total work units (for all statData)
    int progress = 0;

    // Iterate over each statData
    for (int s = 0; s < vizData->statsData.size(); ++s) {
        VizData::StatsData& stats = vizData->statsData[s];

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

        QVector<QFuture<void>> futures; // Store futures to wait for all threads to finish

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

                // Run percentile calculations in parallel using QtConcurrent::run()
                futures.append(QtConcurrent::run(calculatePercentiles, std::ref(stats), values, month, loc, 50, std::ref(stats.median)));
                futures.append(QtConcurrent::run(calculatePercentiles, std::ref(stats), values, month, loc, 5, std::ref(stats.iqr5)));
                futures.append(QtConcurrent::run(calculatePercentiles, std::ref(stats), values, month, loc, 25, std::ref(stats.iqr25)));
                futures.append(QtConcurrent::run(calculatePercentiles, std::ref(stats), values, month, loc, 75, std::ref(stats.iqr75)));
                futures.append(QtConcurrent::run(calculatePercentiles, std::ref(stats), values, month, loc, 95, std::ref(stats.iqr95)));

                // Increment progress and call the progress callback
                progress += 5;  // Increment by 5 because we launched 5 tasks
                if (progressCallback) {
                    int percentProgress = (progress * 100) / totalIterations;
                    progressCallback(percentProgress);  // Report percentage progress, normalized to 0-100
                }

                // Update global min/max values from data
                double localMin = *std::min_element(values.begin(), values.end());
                double localMax = *std::max_element(values.begin(), values.end());
                globalMin = qMin(globalMin, localMin);
                globalMax = qMax(globalMax, localMax);
            }
        }

        // Wait for all threads to complete
        for (auto& future : futures) {
            future.waitForFinished();
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
