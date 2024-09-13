#include "loadersqlite.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QDebug>
#include <QSqlRecord>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QApplication>

#include "vizdata.h"

void LoaderSQLite::loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Load a single file
    qDebug() << "Error: Not implemented loadFileSingle for LoaderSQLite!";
}

void LoaderSQLite::loadFileList(const QStringList &dbPathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Load a list of files
    qDebug() << "Error: Not implemented loadFileList for LoaderSQLite!";
}

void processDatabase(const QString &dbPath, int dbIndex, const QString &locationID, const QString &monthID,
                     const QStringList &columnList, const QString &databaseName, VizData *vizData) {
    int numLocations = vizData->rasterData->locationRaster;
    int numMonths = vizData->monthCountStartToEnd;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", QString::number(dbIndex));
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Failed to open database:" << dbPath << db.lastError().text();
        return;
    }

    QString queryStr = QString("SELECT %1, %2, %3 FROM %4")
                           .arg(locationID)
                           .arg(monthID)
                           .arg(columnList.join(','))
                           .arg(databaseName);

    QSqlQuery query(db);
    if (!query.exec(queryStr)) {
        qDebug() << "Failed to execute query:" << queryStr << query.lastError().text();
        db.close();
        return;
    }

    while (query.next()) {
        int loc = query.value(0).toInt();
        int month = query.value(1).toInt() - 1;

        if (loc >= 0 && loc < numLocations && month >= 0 && month < numMonths) {
            for (int colIndex = 0; colIndex < columnList.size(); ++colIndex) {
                double value = query.value(2 + colIndex).toDouble();
                vizData->statsData[colIndex].data[dbIndex][loc][month] = value;
            }
        }
    }

    db.close();
}

void LoaderSQLite::loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString databaseName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    int numDatabases = dbPathList.size();
    QStringList columnList = columns.split(',');

    // Initialize statsData for each column
    vizData->statsData.clear();
    for (int i = 0; i < columnList.size(); ++i) {
        VizData::StatsData stats;
        stats.data.resize(numDatabases);
        for (int dbIndex = 0; dbIndex < numDatabases; ++dbIndex) {
            stats.data[dbIndex].resize(vizData->rasterData->locationRaster);
            for (int locIndex = 0; locIndex < vizData->rasterData->locationRaster; ++locIndex) {
                stats.data[dbIndex][locIndex].resize(vizData->monthCountStartToEnd);
            }
        }
        vizData->statsData.append(stats);
    }

    // Progress tracking
    QVector<QFuture<void>> futures;

    // Launch each database processing task concurrently
    for (int dbIndex = 0; dbIndex < numDatabases; ++dbIndex) {
        QFuture<void> future = QtConcurrent::run([=]() {
            processDatabase(dbPathList[dbIndex], dbIndex, locationID, monthID, columnList, databaseName, vizData);
        });
        futures.append(future);
    }

    // Poll the progress of the futures periodically
    int progress = 0;
    while (progress < numDatabases) {
        int currentProgress = 0;

        // Check how many futures have finished
        for (auto &future : futures) {
            if (future.isFinished()) {
                currentProgress++;
            }
        }

        // If progress has updated, call the progress callback
        if (currentProgress > progress) {
            progress = currentProgress;
            if (progressCallback) {
                int percentComplete = progress * 100 / numDatabases;
                progressCallback(percentComplete);
            }
        }

        // Yield control to avoid blocking the thread
        QCoreApplication::processEvents(); // Yield to allow the GUI or other tasks to process
    }

    // Once all tasks are finished, call the completion callback
    if (completionCallback) {
        completionCallback();
    }

}














