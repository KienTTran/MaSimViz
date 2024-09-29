#ifndef VIZDATA_H
#define VIZDATA_H

#include <QList>
#include <QColor>
#include <QVector3D>
#include <QDateTime>

#include "ascfile.h"
#include "preference.h"

class VizData
{
public:

    struct ChatbotData{
        bool isWithAPI = false;
        QString apiProvider = "";
        QStringList apiProviders = {"OpenAI",
                                    "Llamacpp"};
        QMap<QString,QStringList> apiProviderInfo;
        QString modelPath = "";
    };

    struct RasterData{
        AscFile *raster;
        int nLocations;
        double dataMin = 0.0;
        double dataMax = 0.0;
        QMap<int, QPair<int, int>> locationPair1DTo2D;
        QMap<QPair<int, int>, int> locationPair2DTo1D;
        QMap<QPair<int, int>, int> locationPair2DTo1DDistrict;
    };
    //template struct
    struct StatsData{
        QList<QList<QList<double>>> data;        
        QList<QList<QList<double>>> iqr;
        QList<int> iqrRanges = {50, 5, 25, 75, 95};
        double dataMin = 0.0;
        double dataMax = 0.0;
        double medianMin = 0.0;
        double medianMax = 0.0;
    };

    struct SQLData{
        QList<QString> dbPaths;
        QList<QString> dbTables;
        QMap<int,QMap<int,QString>> dbColumns;
        QString locationID = "locationid";
        QString monthID = "monthlydataid";
        QMap<QString,QString> tableColumnsMap;
    };


    QVector3D interpolate(int lowerStep, float factor){
        return interpolateColors(colorMap[lowerStep], colorMap[lowerStep + 1], factor);
    }

    QVector3D interpolateColors(const QVector3D& color1, const QVector3D& color2, float factor) {
        return (1.0f - factor) * color1 + factor * color2;
    }

    // Define extended color stops for a more detailed gradient
    QVector<QVector3D> colorMap = {
        QVector3D(0.1f, 0.1f, 0.1f),  // Gray
        QVector3D(0.0f, 0.25f, 0.0f),  // Midpoint between Blue and Light Blue
        QVector3D(0.0f, 0.5f, 1.0f),  // Light Blue
        QVector3D(0.0f, 0.75f, 1.0f),  // Midpoint between Light Blue and Cyan
        QVector3D(0.0f, 1.0f, 1.0f),  // Cyan
        QVector3D(0.0f, 1.0f, 0.75f),  // Midpoint between Cyan and Light Green-Cyan
        QVector3D(0.0f, 1.0f, 0.5f),  // Light Green-Cyan
        QVector3D(0.0f, 1.0f, 0.25f),  // Midpoint between Light Green-Cyan and Green
        QVector3D(0.0f, 1.0f, 0.0f),  // Green
        QVector3D(0.25f, 1.0f, 0.0f),  // Midpoint between Green and Yellow-Green
        QVector3D(0.5f, 1.0f, 0.0f),  // Yellow-Green
        QVector3D(0.75f, 1.0f, 0.0f),  // Midpoint between Yellow-Green and Yellow
        QVector3D(1.0f, 1.0f, 0.0f),  // Yellow
        QVector3D(1.0f, 0.75f, 0.0f),  // Midpoint between Yellow and Orange
        QVector3D(1.0f, 0.5f, 0.0f),  // Orange
        QVector3D(1.0f, 0.25f, 0.0f),  // Midpoint between Orange and Red
        QVector3D(1.0f, 0.0f, 0.0f),  // Red
        QVector3D(0.75f, 0.0f, 0.25f),  // Midpoint between Red and Magenta-Purple
        QVector3D(0.5f, 0.0f, 0.5f),  // Magenta-Purple
        QVector3D(0.5f, 0.0f, 0.75f),  // Midpoint between Magenta-Purple and Purple
        QVector3D(0.5f, 0.0f, 1.0f),  // Purple
    };

    VizData();
    QString currentDirectory;
    RasterData *rasterData;
    QDateTime simStartDate;
    QDateTime simCompDate;
    QDateTime simEndDate;
    int monthCountStartToEnd;
    int monthCountCompToEnd;
    QMap<QString,StatsData> statsData;
    SQLData sqlData;
    bool isDistrictReporter = false;
    ChatbotData chatbotData;
    Preference *prefData;
};

#endif // VIZDATA_H
