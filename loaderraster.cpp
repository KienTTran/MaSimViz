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

    int nrows = -1;
    int ncols = -1;
    double xllcorner = 0.0;
    double yllcorner = 0.0;
    double cellsize = 0.0;
    double nodata_value = 0.0;

    // Read the header information
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.startsWith("ncols")) {
            ncols = line.split(" ", Qt::SkipEmptyParts).last().toInt();
        } else if (line.startsWith("nrows")) {
            nrows = line.split(" ", Qt::SkipEmptyParts).last().toInt();
        } else if (line.startsWith("xllcorner")) {
            xllcorner = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else if (line.startsWith("yllcorner")) {
            yllcorner = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else if (line.startsWith("cellsize")) {
            cellsize = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else if (line.startsWith("NODATA_value")) {
            nodata_value = line.split(" ", Qt::SkipEmptyParts).last().toDouble();
        } else {
            // Break when grid data starts, typically after headers
            break;
        }
    }

    // Allocate memory for raster data
    float** data = new float*[nrows];
    for (int i = 0; i < nrows; ++i) {
        data[i] = new float[ncols];
    }

    // Now read the grid data
    int row = 0;
    while (!in.atEnd() && row <= nrows) {
        line = in.readLine();
        if (!line.isEmpty()) {
            QStringList rowElements = line.split(" ", Qt::SkipEmptyParts);
            for (int col = 0; col < rowElements.size() && col < ncols; ++col) {
                data[row][col] = rowElements[col].toFloat();
            }
            row++;
        }
    }

    file.close();

    // Store the metadata and grid data into VizData
    vizData->rasterData->ncols = ncols;
    vizData->rasterData->nrows = nrows;
    vizData->rasterData->xllcorner = xllcorner;
    vizData->rasterData->yllcorner = yllcorner;
    vizData->rasterData->cellsize = cellsize;
    vizData->rasterData->nodata_value = nodata_value;

    // Clear existing data
    vizData->rasterData->values.clear();

    // Copy grid data into VizData
    for (int i = 0; i < nrows; ++i) {
        QList<double> rowValues;
        for (int j = 0; j < ncols; ++j) {
            rowValues.append(static_cast<double>(data[i][j]));
        }
        vizData->rasterData->values.append(rowValues);
    }

    qDebug() << "ncols: " << ncols << " nrows: " << nrows << " xllcorner: " << xllcorner << " yllcorner: " << yllcorner << " cellsize: " << cellsize << " nodata_value: " << nodata_value;
    qDebug() << vizData->rasterData->values.size() << " " << vizData->rasterData->values[0].size();

    // Free the allocated memory for data
    for (int i = 0; i < nrows; ++i) {
        delete[] data[i];
    }
    delete[] data;

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
    int numRasters = filePathList.size();
    int N = (numRasters + 2) / 3;  // This gives us the correct number of rows in the grid.

    int individualCols = 0, individualRows = 0;

    // Placeholder to store raster data as it's loaded
    QList<QList<QList<double>>> rasterChunks; // Store 2D grid for each raster

    // Iterate through the list of rasters
    for (int i = 0; i < filePathList.size(); ++i) {
        // Load individual raster data
        loadFileSingle(filePathList[i], vizData, nullptr, nullptr);

        if (vizData->rasterData->ncols == 0 || vizData->rasterData->nrows == 0) {
            qDebug() << "Error: Invalid raster data in file" << filePathList[i];
            continue;
        }

        // Initialize individual raster dimensions (only for the first raster)
        if (i == 0) {
            individualCols = vizData->rasterData->ncols;  // For example, 96
            individualRows = vizData->rasterData->nrows;  // For example, 131
        }

        // Store raster data for later combining
        rasterChunks.append(vizData->rasterData->values);
    }

    // Prepare the dimensions of the combined raster
    int combinedCols = individualCols * 3;    // Grid will always have 3 columns (3 rasters wide)
    int combinedRows = individualRows * N;    // Grid has N rows, where N is the number of rows (ceil(numRasters / 3))

    qDebug() << "Combined raster grid will be" << combinedCols << "x" << combinedRows;
    qDebug() << "Individual raster size:" << individualCols << "x" << individualRows;
    qDebug() << "Number of rasters:" << numRasters;
    qDebug() << "Number of rows in combined grid:" << N;


    // Initialize the combined raster data
    vizData->rasterDataAll->ncols = combinedCols;
    vizData->rasterDataAll->nrows = combinedRows;
    vizData->rasterDataAll->xllcorner = vizData->rasterData->xllcorner; // You can modify this based on your requirement
    vizData->rasterDataAll->yllcorner = vizData->rasterData->yllcorner; // You can modify this as needed
    vizData->rasterDataAll->cellsize = vizData->rasterData->cellsize;
    vizData->rasterDataAll->nodata_value = vizData->rasterData->nodata_value;

    // Initialize combined raster grid with NODATA_value
    vizData->rasterDataAll->values.resize(combinedRows);
    for (int i = 0; i < combinedRows; ++i) {
        vizData->rasterDataAll->values[i].resize(combinedCols, vizData->rasterData->nodata_value);
    }

    // Now fill in the combined raster grid from the individual rasters
    for (int i = 0; i < rasterChunks.size(); ++i) {
        int colOffset = (i % 3) * individualCols;  // Calculate column offset based on the raster's position in the grid
        int rowOffset = (i / 3) * individualRows;  // Calculate row offset

        // Place each raster in the appropriate position in the combined grid
        for (int row = 0; row < individualRows; ++row) {
            for (int col = 0; col < individualCols; ++col) {
                vizData->rasterDataAll->values[rowOffset + row][colOffset + col] = rasterChunks[i][row][col];
            }
        }
    }

    qDebug() << "Combined raster grid created with size:" << combinedCols << "x" << combinedRows;
}







