#include "loaderyml.h"
#include <QDateTime>
#include <QString>
#include <regex>

QDateTime LoaderYML::parseDate(const std::string dateStdStr) {
    // Define regex patterns for date formats
    std::regex format1("^\\d{4}/\\d{2}/\\d{2}$"); // yyyy/MM/dd
    std::regex format2("^\\d{4}/\\d{1}/\\d{1}$"); // yyyy/M/d
    std::regex format3("^\\d{4}/\\d{2}/\\d{1}$"); // yyyy/MM/d
    std::regex format4("^\\d{4}/\\d{1}/\\d{2}$"); // yyyy/M/dd

    QString dateStr = dateStdStr.c_str();

    if (std::regex_match(dateStdStr, format1)) {
        return QDateTime::fromString(dateStr, "yyyy/MM/dd");
    } else if (std::regex_match(dateStdStr, format2)) {
        return QDateTime::fromString(dateStr, "yyyy/M/d");
    } else if (std::regex_match(dateStdStr, format3)) {
        return QDateTime::fromString(dateStr, "yyyy/MM/d");
    } else if (std::regex_match(dateStdStr, format4)) {
        return QDateTime::fromString(dateStr, "yyyy/M/dd");
    } else {
        // Return invalid QDateTime if the format does not match
        return QDateTime();
    }
}

int LoaderYML::monthsBetween(const QDateTime& start, const QDateTime& end) {
    // Get the QDate from QDateTime
    QDate startDate = start.date();
    QDate endDate = end.date();

    // Calculate the total number of months between the two dates
    int yearsDiff = endDate.year() - startDate.year();
    int monthsDiff = endDate.month() - startDate.month();

    // Total months calculation: (years * 12) + months
    int totalMonths = yearsDiff * 12 + monthsDiff;

    return totalMonths;
}

// Function to check if a year is a leap year
bool isLeapYear(int year) {
    return QDate::isLeapYear(year);
}

void LoaderYML::loadFileSingle(const QString &filePath, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Load YML file, first file only
    YAML::Node config = YAML::LoadFile(filePath.toStdString());

    vizData->simStartDate = parseDate(config["starting_date"].as<std::string>());
    vizData->simCompDate = parseDate(config["start_of_comparison_period"].as<std::string>());
    vizData->simEndDate = parseDate(config["ending_date"].as<std::string>());

    qDebug() << "simStartDate: " << vizData->simStartDate;
    qDebug() << "simCompDate: " << vizData->simCompDate;
    qDebug() << "simEndDate: " << vizData->simEndDate;

    vizData->monthCountStartToEnd = monthsBetween(vizData->simStartDate, vizData->simEndDate) + 1;
    vizData->monthCountCompToEnd = monthsBetween(vizData->simStartDate, vizData->simCompDate) + 1;

    qDebug() << "monthCountStartToEnd: " << vizData->monthCountStartToEnd;
    qDebug() << "monthCountCompToEnd: " << vizData->monthCountCompToEnd;
}

void LoaderYML::loadDBList(const QStringList &dbPathList, const QString locationID, const QString monthID, const QString columns, const QString tableName, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback) {
    // Load a single file
    qDebug() << "Error: Not implemented loadDBList for LoaderYML!";
}

void LoaderYML::loadFileList(const QStringList &filePathList, VizData *vizData, std::function<void(int)> progressCallback, std::function<void()> completionCallback){
    // Load a loadFileList
    qDebug() << "Error: Not implemented loadFileList for LoaderYML!";
}
