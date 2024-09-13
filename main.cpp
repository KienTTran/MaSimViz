#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    // Set the OpenGL version and profile to 4.1 (max supported on macOS)
    QSurfaceFormat format;
    format.setVersion(4, 1);  // OpenGL 4.1
    format.setProfile(QSurfaceFormat::CoreProfile);  // Core profile
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MaSimPlayer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
