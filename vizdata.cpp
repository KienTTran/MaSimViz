#include "vizdata.h"

VizData::VizData() {
    rasterData = new RasterData();
    rasterData->values = QList<QList<double>>();
    rasterData->ncols = 0;
    rasterData->nrows = 0;
    rasterData->xllcorner = 0.0;
    rasterData->yllcorner = 0.0;
    rasterData->cellsize = 0.0;
    rasterData->nodata_value = -9999;

    rasterDataAll = new RasterData();
    rasterDataAll->values = QList<QList<double>>();
    rasterDataAll->ncols = 0;
    rasterDataAll->nrows = 0;
    rasterDataAll->xllcorner = 0.0;
    rasterDataAll->yllcorner = 0.0;
    rasterDataAll->cellsize = 0.0;
    rasterDataAll->nodata_value = -9999;

    statsData = QList<StatsData>();
}
