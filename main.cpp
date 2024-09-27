#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
 #include <QStyleFactory>
#include <QSysInfo>

int main(int argc, char *argv[])
{
    // Set the OpenGL version and profile to 4.1 (max supported on macOS)
    QSurfaceFormat format;
    format.setVersion(4, 1);  // OpenGL 4.1
    format.setProfile(QSurfaceFormat::CoreProfile);  // Core profile
    QSurfaceFormat::setDefaultFormat(format);
    QApplication a(argc, argv);


    // Set app palette based on system theme
    QPalette palette;
    bool isDarkTheme = false;

#ifdef Q_OS_MAC
    // Use QSettings to detect dark mode on macOS
    QSettings settings("Apple", "Global Preferences");
    isDarkTheme = settings.value("AppleInterfaceStyle", "").toString() == "Dark";
#endif

#ifdef Q_OS_WIN
    // On Windows 10 and above, dark mode can be detected from registry
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    isDarkTheme = settings.value("AppsUseLightTheme", 1).toInt() == 0;
#endif

    if (isDarkTheme) {
        palette.setColor(QPalette::Window, QColor(45, 45, 48));
        palette.setColor(QPalette::WindowText, Qt::white);
    } else {
        palette.setColor(QPalette::Window, Qt::white);
        palette.setColor(QPalette::WindowText, QColor(45, 45, 48));
    }

    a.setPalette(palette);

    // Set language translation
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MaSimViz_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // Create and show main window
    MainWindow w;
    w.show();

    return a.exec();
}


