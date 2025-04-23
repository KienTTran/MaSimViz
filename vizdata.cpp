#include "vizdata.h"

#include <QIODevice>
#include <QFile>
#include <QFileInfo>

VizData::VizData() {
    rasterData = new RasterData();
    rasterData->raster = new AscFile();
    statsData = QMap<QString,StatsData>();
    statsDataSummary = QMap<QString,StatsDataSummary>();
    genotypeFrequencies = QList<GenotypeFrequency>();
    prefData = new Preference();
    chatbotData.apiProviderInfo = QMap<QString,QStringList>();
}

