#ifndef VIZDATA_H
#define VIZDATA_H

#include <QList>
#include <QDateTime>

class VizData
{
public:
    struct RasterData{
        int ncols = 0;
        int nrows = 0;
        double xllcorner = 0;
        double yllcorner = 0;
        double cellsize = 0;
        double nodata_value = -9999;
        QVector<QVector<double>> values;
        int locationRaster = 0;
        int locationSim = 0;
        QMap<int, QPair<int, int>> locationPair;
    };
    //template struct
    struct StatsData{
        QList<QList<QList<double>>> data;
        QList<QList<double>> median;
        int min = 0;
        int max = 0;
    };

    VizData();
    RasterData *rasterData;
    QDateTime simStartDate;
    QDateTime simCompDate;
    QDateTime simEndDate;
    int monthCountStartToEnd;
    int monthCountCompToEnd;
    QList<StatsData> statsData;
};

#endif // VIZDATA_H
