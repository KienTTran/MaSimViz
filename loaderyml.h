#ifndef LOADERYML_H
#define LOADERYML_H

#include "loader.h"
#include "yaml-cpp/yaml.h"

class LoaderYML : public Loader
{
public:
    LoaderYML() = default;
    void loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadFileList(const QStringList &dbPathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;

    QDateTime parseDate(const std::string dateStdStr);
    int monthsBetween(const QDateTime& start, const QDateTime& end);
};

#endif // LOADERYML_H
