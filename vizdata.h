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
        QList<QList<double>> iqr5;
        QList<QList<double>> iqr25;
        QList<QList<double>> iqr75;
        QList<QList<double>> iqr95;
        double dataMin = 0;
        double dataMax = 0;
        double medianMin = 0;
        double medianMax = 0;
    };

    struct SQLData{
        QList<QString> dbPaths;
        QList<QString> dbTables;
        QMap<int,QMap<int,QString>> dbColumns;
        QString locationID;
        QString monthID;
        QMap<QString,QString> tableColumnMap;
    };

    VizData();
    RasterData *rasterData;
    QDateTime simStartDate;
    QDateTime simCompDate;
    QDateTime simEndDate;
    int monthCountStartToEnd;
    int monthCountCompToEnd;
    QList<StatsData> statsData;
    SQLData sqlData;
};

#endif // VIZDATA_H
