#include "Preference.h"

// Constructor implementation
Preference::Preference(const QString &filePath, QObject *parent) : QObject(parent) {
    if (!filePath.isEmpty()) {
        // Use a custom file path (.ini file)
        settings = new QSettings(filePath, QSettings::IniFormat);
    } else {
        // Use the default organization and app name
        settings = new QSettings("MyCompany", "MyApp");
    }
}

// Save window size to settings
void Preference::saveWindowSize(const QSize &size) {
    settings->setValue("window/size", size);
}

// Save window position to settings
void Preference::saveWindowPosition(const QPoint &position) {
    settings->setValue("window/position", position);
}

// Save preferred theme to settings
void Preference::savePreferredTheme(const QString &theme) {
    settings->setValue("user/preferredTheme", theme);
}

// Save API key path to settings
void Preference::saveAPIKeyPath(const QString &apiKeyPath) {
    settings->setValue("paths/apiKeyPath", apiKeyPath);
}

// Save model path to settings
void Preference::saveModelPath(const QString &modelPath) {
    settings->setValue("paths/modelPath", modelPath);
}

// Save work path to settings
void Preference::saveWorkPath(const QString &workPath) {
    settings->setValue("paths/workPath", workPath);
}

// Save database path to settings
void Preference::saveDBPaths(const QStringList &dbPath) {
    settings->setValue("paths/dbPath", dbPath);
}

// Load window size from settings
QSize Preference::loadWindowSize(const QSize &defaultSize) {
    return settings->value("window/size", defaultSize).toSize();
}

// Load window position from settings
QPoint Preference::loadWindowPosition(const QPoint &defaultPosition) {
    return settings->value("window/position", defaultPosition).toPoint();
}

// Load preferred theme from settings
QString Preference::loadPreferredTheme(const QString &defaultTheme) {
    return settings->value("user/preferredTheme", defaultTheme).toString();
}

// Load API key path from settings
QString Preference::loadAPIKeyPath(const QString &defaultPath) {
    return settings->value("paths/apiKeyPath", defaultPath).toString();
}

// Load model path from settings
QString Preference::loadModelPath(const QString &defaultPath) {
    return settings->value("paths/modelPath", defaultPath).toString();
}

// Load work path from settings
QString Preference::loadWorkPath(const QString &defaultPath) {
    return settings->value("paths/workPath", defaultPath).toString();
}

// Load database path from settings
QStringList Preference::loadDBPath(const QStringList &defaultPath) {
    return settings->value("paths/dbPath", defaultPath).toStringList();
}
