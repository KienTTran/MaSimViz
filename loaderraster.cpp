#include "loaderraster.h"
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>

LoaderRaster::LoaderRaster() {}

void LoaderRaster::loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Check if the file extension is .asc
    if (!filePath.endsWith(".asc", Qt::CaseInsensitive)) {
        qDebug() << "Error: The file is not an ASC file!";
        return;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Cannot open file!";
        return;
    }

    QTextStream in(&file);
    QString line;

    // Clear the existing raster data
    vizData->rasterData->values.clear();

    // Read the header information
    while (!in.atEnd()) {
        line = in.readLine();

        if (line.startsWith("ncols")) {
            vizData->rasterData->ncols = line.split(" ", Qt::SkipEmptyParts).last().toInt();
        } else if (line.startsWith("nrows")) {
            vizData->rasterData->nrows = line.split(" ", Qt::SkipEmptyParts).last().toInt();
        } else if (line.startsWith("xllcorner")) {
            vizData->rasterData->xllcorner = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else if (line.startsWith("yllcorner")) {
            vizData->rasterData->yllcorner = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else if (line.startsWith("cellsize")) {
            vizData->rasterData->cellsize = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else if (line.startsWith("NODATA_value")) {
            vizData->rasterData->nodata_value = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else {
            // Break when grid data starts, typically after headers
            break;
        }
    }

    // Now read the grid data
    while (!in.atEnd()) {
        line = in.readLine();  // Move this outside the condition
        if (!line.isEmpty()) {
            QList<double> rowValues;
            QStringList rowElements = line.split(" ", Qt::SkipEmptyParts);

            for (const QString &value : rowElements) {
                rowValues.append(value.toDouble());
            }

            vizData->rasterData->values.append(rowValues);
        }
    }

    file.close();
}

void LoaderRaster::loadFileList(const QStringList &dbPathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Load a list of files
    qDebug() << "Error: Not implemented loadFileList for LoaderRaster!";
}

void LoaderRaster::loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Not implemented for raster data
    qDebug() << "Error: Not implemented loadDBList for LoaderRaster!";
}
