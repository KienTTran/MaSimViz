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
    // Load a single database and get all columns
    vizData->sqlData.dbPaths.clear();
    vizData->sqlData.dbTables.clear();
    vizData->sqlData.dbColumns.clear();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filePath);
    if (!db.open()) {
        qDebug() << "Failed to open database:" << filePath << db.lastError().text();
        return;
    }

    // Store the database path in VizData's sqlData
    vizData->sqlData.dbPaths.append(filePath);

    // Get list of tables in the database
    QStringList tables = db.tables();
    if (tables.isEmpty()) {
        qDebug() << "No tables found in database:" << filePath;
        db.close();
        return;
    }

    qDebug() << "Tables found in database:" << filePath;

    // Add the table names to VizData's sqlData
    vizData->sqlData.dbTables.append(tables);

    // Index for the current database in dbPaths (the latest added)
    int dbIndex = vizData->sqlData.dbPaths.size() - 1;

    // Loop through tables and store their columns in VizData
    for (const QString &table : tables) {
        QSqlQuery query(db);

        // Get the column information from the table using PRAGMA table_info
        if (!query.exec(QString("PRAGMA table_info(%1)").arg(table))) {
            qDebug() << "Failed to get table info for" << table << ":" << query.lastError().text();
            continue; // Skip to the next table if there's an error
        }

        // Variable to store the index for the table in the column map
        int tableIndex = vizData->sqlData.dbTables.indexOf(table);

        // Column counter to track the order of columns
        int columnIndex = 0;

        // Loop through each column in the table
        while (query.next()) {
            QString columnName = query.value("name").toString();
            vizData->sqlData.dbColumns[tableIndex][columnIndex] = columnName;
            columnIndex++;
        }
    }

    // Clean up and close the database
    db.close();

    // Call the completion callback if provided
    if (completionCallback) {
        completionCallback();
    }
}



void LoaderSQLite::loadFileList(const QStringList &filePathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Load a list of files
    qDebug() << "Error: Not implemented loadFileList for LoaderSQLite!";
}

void processDatabase(const QString &dbPath, int dbIndex, const QString &locationID, const QString &monthID,
                     const QStringList &columnList, const QString &databaseName, VizData *vizData) {
    int numLocations = vizData->rasterData->nLocations;
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
                vizData->statsData[columnList[colIndex]].data[dbIndex][loc][month] = value;
            }
        }
    }

    db.close();
}

void LoaderSQLite::loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    int numDatabases = dbPathList.size();
    if(columns.isEmpty()) {
        qDebug() << "Error: No columns selected!";
        return;
    }
    else if(tableName.isEmpty()) {
        qDebug() << "Error: No table selected!";
        return;
    }
    else if(locationID.isEmpty()) {
        qDebug() << "Error: No location ID selected!";
        return;
    }
    else if(monthID.isEmpty()) {
        qDebug() << "Error: No month ID selected!";
        return;
    }
    else{
        qDebug() << "[LoadDBList]Columns: " << columns;
        qDebug() << "[LoadDBList]Table: " << tableName;
        qDebug() << "[LoadDBList]Location ID: " << locationID;
        qDebug() << "[LoadDBList]Month ID: " << monthID;
    }
    QStringList columnList = QStringList();
    if(columns.contains(',')){
        columnList = columns.split(',');
    }
    else{
        columnList.append(columns);
    }

    // Initialize statsData for each column
    vizData->statsData.clear();
    for (int i = 0; i < columnList.size(); ++i) {
        VizData::StatsData stats;
        stats.data.resize(numDatabases);
        for (int dbIndex = 0; dbIndex < numDatabases; ++dbIndex) {
            stats.data[dbIndex].resize(vizData->rasterData->nLocations);
            for (int locIndex = 0; locIndex < vizData->rasterData->nLocations; ++locIndex) {
                stats.data[dbIndex][locIndex].resize(vizData->monthCountStartToEnd);
            }
        }
        vizData->statsData[columnList[i]] = stats;
    }

    // Progress tracking
    QVector<QFuture<void>> futures;

    // Launch each database processing task concurrently
    for (int dbIndex = 0; dbIndex < numDatabases; ++dbIndex) {
        QFuture<void> future = QtConcurrent::run([=]() {
            processDatabase(dbPathList[dbIndex], dbIndex, locationID, monthID, columnList, tableName, vizData);
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














