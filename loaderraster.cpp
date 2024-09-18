#include "loaderraster.h"
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "ascfile.h"

LoaderRaster::LoaderRaster() {}

void LoaderRaster::loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Check if the file extension is .asc
    if (!filePath.endsWith(".asc", Qt::CaseInsensitive)) {
        qDebug() << "Error: The file is not an ASC file!";
        return;
    }

    vizData->rasterData->raster = AscFileManager::read(filePath.toStdString());

    double min = std::numeric_limits<double>::max();
    double max = -std::numeric_limits<double>::max();

    int locationIndex = 0;
    for(int i = 0; i < vizData->rasterData->raster->NROWS; i++) {
        for(int j = 0; j < vizData->rasterData->raster->NCOLS; j++) {
            if(vizData->rasterData->raster->data[i][j] == vizData->rasterData->raster->NODATA_VALUE) {
                continue;
            }
            min = qMin(min, vizData->rasterData->raster->data[i][j]);
            max = qMax(max, vizData->rasterData->raster->data[i][j]);
            vizData->rasterData->locationPair1DTo2D[locationIndex] = std::make_pair(i, j);
            vizData->rasterData->locationPair2DTo1D[QPair<int,int>(i,j)] = locationIndex;
            locationIndex++;
        }
    }

    vizData->rasterData->dataMin = min;
    vizData->rasterData->dataMax = max;

    vizData->rasterData->nLocations = locationIndex;

    qDebug() << "Raster data loaded from file:" << filePath;
    qDebug() << "Number of columns:" << vizData->rasterData->raster->NCOLS;
    qDebug() << "Number of rows:" << vizData->rasterData->raster->NROWS;
    qDebug() << "XLLCENTER:" << vizData->rasterData->raster->XLLCENTER;
    qDebug() << "YLLCENTER:" << vizData->rasterData->raster->YLLCENTER;
    qDebug() << "XLLCORNER:" << vizData->rasterData->raster->XLLCORNER;
    qDebug() << "YLLCORNER:" << vizData->rasterData->raster->YLLCORNER;
    qDebug() << "CELLSIZE:" << vizData->rasterData->raster->CELLSIZE;
    qDebug() << "NODATA_VALUE:" << vizData->rasterData->raster->NODATA_VALUE;
    qDebug() << "Number of locations:" << locationIndex;
    qDebug() << "Data min:" << vizData->rasterData->dataMin;
    qDebug() << "Data max:" << vizData->rasterData->dataMax;

    // Trigger the completion callback
    if (completionCallback) {
        completionCallback();
    }
}


void LoaderRaster::loadFileList(const QStringList &dbPathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Load a list of files
    qDebug() << "Error: Not implemented loadFileList for LoaderRaster!";
}

void LoaderRaster::loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Not implemented for raster data
    qDebug() << "Error: Not implemented loadDBList for LoaderRaster!";
}

void LoaderRaster::combinedRasterData(const QStringList &filePathList, VizData *vizData) {
}







