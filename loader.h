#ifndef LOADER_H
#define LOADER_H
#include <QString>
#include "vizdata.h"

#include <QObject>

class Loader : public QObject
{
    Q_OBJECT
public:
    Loader();

public:
    virtual void loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) = 0;
    virtual void loadFileList(const QStringList &filePathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) = 0;
    virtual void loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) = 0;


};

#endif // LOADER_H
