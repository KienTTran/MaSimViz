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
    statsData = QList<StatsData>();
}
