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

    // Save functions
    Q_INVOKABLE void saveWindowSize(const QSize &size);
    Q_INVOKABLE void saveWindowPosition(const QPoint &position);
    Q_INVOKABLE void savePreferredTheme(const QString &theme);

    // Save paths
    Q_INVOKABLE void saveAPIKeyPath(const QString &apiKeyPath);
    Q_INVOKABLE void saveModelPath(const QString &modelPath);
    Q_INVOKABLE void saveWorkPath(const QString &workPath);
    Q_INVOKABLE void saveDBPaths(const QStringList &dbPath);

    // Load functions
    Q_INVOKABLE QSize loadWindowSize(const QSize &defaultSize = QSize(1024, 768));
    Q_INVOKABLE QPoint loadWindowPosition(const QPoint &defaultPosition = QPoint(200, 200));
    Q_INVOKABLE QString loadPreferredTheme(const QString &defaultTheme = "dark");

    // Load paths
    Q_INVOKABLE QString loadAPIKeyPath(const QString &defaultPath = QString());
    Q_INVOKABLE QString loadModelPath(const QString &defaultPath = QString());
    Q_INVOKABLE QString loadWorkPath(const QString &defaultPath = QString());
    Q_INVOKABLE QStringList loadDBPath(const QStringList &defaultPath = QStringList());

private:
    QSettings *settings;
};

#endif // PREFERENCE_H
