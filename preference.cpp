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

QString Preference::getConfigFilePath(){
    return settings->fileName();
}

// Save chatbot API provider to settings
void Preference::saveChatbotAPIProvider(const QString &chatbotAPIProvider) {
    settings->setValue("ChatbotAPIInfo/apiProvider", chatbotAPIProvider);
}

// Load chatbot API provider from settings
QString Preference::loadChatbotAPIProvider(const QString &defaultPath) {
    return settings->value("ChatbotAPIInfo/apiProvider", defaultPath).toString();
}

// Load chatbot API info from settings
QMap<QString, QStringList> Preference::loadChatbotAPIInfo(const QMap<QString, QStringList> &defaultData) {
    QMap<QString, QStringList> chatbotAPIInfo;

    // Check if the "ChatbotAPIInfo" group exists in settings
    if (!settings->childGroups().contains("ChatbotAPIInfo")) {
        // If the group doesn't exist, return the default data
        return defaultData;
    }

    settings->beginGroup("ChatbotAPIInfo");
    QStringList keys = settings->childKeys();
    foreach (QString key, keys) {
        chatbotAPIInfo[key] = settings->value(key).toStringList();
    }
    settings->endGroup();

    return chatbotAPIInfo;
}


// Save chatbot API info to settings
void Preference::saveChatbotAPIInfo(const QMap<QString, QStringList> &chatbotAPIInfo) {
    settings->beginGroup("ChatbotAPIInfo");
    QMap<QString, QStringList>::const_iterator i = chatbotAPIInfo.constBegin();
    while (i != chatbotAPIInfo.constEnd()) {
        settings->setValue(i.key(), i.value());
        ++i;
    }
    settings->endGroup();
}
