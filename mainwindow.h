#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QList>
#include <QtConcurrent/QtConcurrent>

#include <QtConcurrent>
#include <QFuture>

#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <atomic>
#include <tuple>

#include "loader.h"
#include "vizdata.h"
#include "graphicsviewcustom.h"
#include "glwidgetcustom.h"
#include "dataprocessor.h"
#include "chartcustom.h"

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

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_bt_auto_load_folder_clicked();

    void on_cb_raster_list_currentIndexChanged(int index);

    void on_bt_process_clicked();

    void on_bt_run_clicked();

    void on_le_sim_path_returnPressed();

    void on_slider_progress_valueChanged(int value);

    void on_slider_progress_sliderMoved(int position);

    void on_cb_db_list_currentIndexChanged(int index);

    void onMouseMoved(const QPoint &pos);

public slots:
    void onSquareClicked(const QPoint &pos, const QColor &color);

private:
    Ui::MainWindow *ui;
    QString statusMessage = "";
    Loader *loader;
    QStringList ascFileList;
    QStringList dbFileList;
    QStringList ymlFileList;
    QStringList csvFileList;
    QGraphicsScene *scene;
    VizData *vizData;
    DataProcessor *dataProcessor;
    ChartCustom *chart;

private:
    void disabeInputWidgets();
    void enableInputWidgets();
    void showWhenPlay();
    void showWhenPause();
    void displayDataInTable(int col, int row);

private:
    bool all_rasters_exist = false;

    std::atomic<bool> stopLoop;  // Global or class member to stop the loop
    bool isRunning = false;             // Global or class member to track play/pause state
    int currentMonth = 0;
    int currentColIndexPlaying = 0;
    QMap<int,QColor> currentLocationSelectedMap;
    QPair<int,int> currenColRowSelected = qMakePair(0,0);

    bool inspectMode = false;


};
#endif // MAINWINDOW_H
