#ifndef LOADERSQLITE_H
#define LOADERSQLITE_H
#include <QtSql>
#include <QDebug>
#include <QList>
#include "loader.h"

class LoaderSQLite: public Loader
{
public:
    LoaderSQLite() = default;

public:
    void loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadFileList(const QStringList &dbPathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString databaseName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;

};

#endif // LOADERSQLITE_H
