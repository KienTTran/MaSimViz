#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>

#include <QtConcurrent/QtConcurrent>

#include <QtConcurrent>
#include <QFuture>

#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include "loader.h"
#include "vizdata.h"
#include "graphicsviewcustom.h"
#include "glwidgetcustom.h"
#include "dataprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_bt_auto_load_folder_clicked();

    void on_cb_raster_list_activated(int index);

    void on_cb_raster_list_currentIndexChanged(int index);

    void on_bt_process_clicked();

    void on_bt_run_clicked();

    void on_le_sim_path_returnPressed();

private:
    Ui::MainWindow *ui;
    QString statusMessage = "";
    Loader *loader;
    QStringList ascFileList;
    QStringList dbFileList;
    QStringList ymlFileList;
    QStringList csvFileList;
    QList<GraphicsViewCustom*> graphicsViewCustomList;
    QList<QGraphicsScene*> graphicsSceneList;
    QList<QSlider*> zoomSliderList;
    GLWidgetCustom *glWidgetCustom;
    VizData *vizData;
    DataProcessor *dataProcessor;

private:
    void disabeInputWidgets();
    void enableInputWidgets();

private:
    bool all_rasters_exist = false;
};
#endif // MAINWINDOW_H
