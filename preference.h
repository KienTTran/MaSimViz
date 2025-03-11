#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QSettings>
#include <QObject>
#include <QString>
#include <QSize>
#include <QPoint>

class Preference : public QObject {
    Q_OBJECT

public:
    // Constructor: Initializes with a file path or organization and app name
    explicit Preference(const QString &filePath = QString(), QObject *parent = nullptr);

    // Save
    Q_INVOKABLE void saveWindowSize(const QSize &size);
    Q_INVOKABLE void saveWindowPosition(const QPoint &position);
    Q_INVOKABLE void savePreferredTheme(const QString &theme);
    Q_INVOKABLE void saveModelPath(const QString &modelPath);
    Q_INVOKABLE void saveWorkPath(const QString &workPath);
    Q_INVOKABLE void saveDBPaths(const QStringList &dbPath);
    Q_INVOKABLE void saveChatbotAPIProvider(const QString &chatbotAPIProvider);
    Q_INVOKABLE void saveChatbotAPIInfo(const QMap<QString, QStringList> &chatbotAPIInfo);

    // Load
    Q_INVOKABLE QSize loadWindowSize(const QSize &defaultSize = QSize(1024, 768));
    Q_INVOKABLE QPoint loadWindowPosition(const QPoint &defaultPosition = QPoint(200, 200));
    Q_INVOKABLE QString loadPreferredTheme(const QString &defaultTheme = "dark");
    Q_INVOKABLE QString loadModelPath(const QString &defaultPath = QString());
    Q_INVOKABLE QString loadWorkPath(const QString &defaultPath = QString());
    Q_INVOKABLE QStringList loadDBPath(const QStringList &defaultPath = QStringList());
    Q_INVOKABLE QString loadChatbotAPIProvider(const QString &defaultPath = QString());
    Q_INVOKABLE QMap<QString, QStringList> loadChatbotAPIInfo(const QMap<QString, QStringList> &defaultData = QMap<QString, QStringList>());

    Q_INVOKABLE QString getConfigFilePath();

private:
    QSettings *settings;
};

#endif // PREFERENCE_H
