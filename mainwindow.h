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

#include "preference.h"
#include "loader.h"
#include "vizdata.h"
#include "graphicsviewcustom.h"
#include "glwidgetcustom.h"
#include "dataprocessor.h"
#include "chartcustom.h"
#include "chatbotwithapi.h"
#include "chatbotwithmodel.h"
#include "webengineviewcustom.h"

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
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_bt_auto_load_folder_clicked();

    void on_bt_process_clicked();

    void on_bt_run_clicked();

    void on_le_sim_path_returnPressed();

    void on_slider_progress_valueChanged(int value);

    void on_slider_progress_sliderMoved(int position);

    void onMouseMoved(const QPoint &pos);

    void on_cb_data_list_currentTextChanged(const QString &arg1);

    void on_slider_progress_sliderPressed();

    void on_slider_progress_sliderReleased();

    void on_bt_chat_setting_clicked();

    void on_chb_assist_clicked(bool checked);

public slots:
    void onSquareClicked(const QPoint &pos, const QColor &color);
    void onChatbotReplyReceived(QString response);

private:
    Ui::MainWindow *ui;
    QString statusMessage = "";
    Preference *preference;
    Loader *loader;
    QStringList ascFileList;
    QStringList dbFileList;
    QStringList ymlFileList;
    QStringList csvFileList;
    QString districtRasterPath;
    QGraphicsScene *scene;
    VizData *vizData;
    DataProcessor *dataProcessor;
    ChartCustom *chart;
    ChatbotWithAPI *onlineChatbot;
    ChatbotWithModel *offlineChatbot;

private:
    bool displaySqlSelection(VizData* vizData, QWidget* parentWidget = nullptr);
    bool displayChatbotSetting(VizData* vizData, QWidget* parentWidget = nullptr);
    void checkDirectory(QString path);
    void disabeInputWidgets();
    void enableInputWidgets(int screenNumber);
    void showMap(QString name);
    void updateMedianMap();
    void resetMedianMap();
    void hideMedianItems();
    void resetPlayState();
    void showLastSquareValue();
    void showChart();
    void saveStatsData();
    void processAndSaveStatsData();
    void loadStatsData(QString tableName);
    void showItemScreenNumber(int screenNumber);
    QString getAPIKeyOrFile(const QString &apiKeyOrFile);

private:
    bool all_rasters_exist = false;
    std::atomic<bool> isRunning = false;             // Global or class member to track play/pause state
    int currentMonth = 0;
    QString currentColNameShown = "";
    QMap<QPair<int,int>,QColor> currentLocationSelectedMap;
    QMap<QString,QString> cbItemPathMap;
    int screenNumber = 0;

signals:
    void addClearButton(bool show);
    void appendChatBotText(QString text);
    void isAssisantReady(bool ready);
};
#endif // MAINWINDOW_H
