#include "vizdata.h"

VizData::VizData() {
    rasterData = new RasterData();
    rasterData->raster = new AscFile();
    statsData = QMap<QString,StatsData>();
}
