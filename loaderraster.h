#ifndef LOADERRASTER_H
#define LOADERRASTER_H

#include "loader.h"

class LoaderRaster : public Loader
{
public:
    LoaderRaster();
    void loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadFileList(const QStringList &dbPathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString databaseName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
};

#endif // LOADERRASTER_H
