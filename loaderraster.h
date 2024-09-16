#ifndef LOADERRASTER_H
#define LOADERRASTER_H

#include "loader.h"

class LoaderRaster : public Loader
{
public:
    LoaderRaster();
    void loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadFileList(const QStringList &filePathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;
    void loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) override;

public:
    void combinedRasterData(const QStringList &filePathList, VizData *vizData);
};

#endif // LOADERRASTER_H
