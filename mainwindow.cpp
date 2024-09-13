#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QDirIterator>
#include <QDebug>
#include <QMessageBox>
#include <QStringListModel>
#include <QGraphicsEllipseItem>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFutureWatcher>

#include "loadersqlite.h"
#include "loaderyml.h"
#include "loaderraster.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    vizData = new VizData();

    ui->le_sim_path->setText("/Users/ktt/Downloads/0ATest_input");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Function to search for .asc files in the given directory
QStringList searchForFilesWithPattern(const QString &directoryPath, QString pattern) {
    QDirIterator it(directoryPath, QStringList() << pattern, QDir::Files, QDirIterator::Subdirectories);
    QStringList foundFiles;
    // Iterate through all found .asc files
    while (it.hasNext()) {
        QString filePath = it.next();
        qDebug() << "Found:" << filePath;
        foundFiles.append(filePath);
    }
    return foundFiles;
}

void MainWindow::on_bt_auto_load_folder_clicked()
{
    statusMessage = "No folder selected.";
    QString defaultDir;

// Check the operating system and set the default directory accordingly
#if defined(Q_OS_WIN)
    defaultDir = "C:\\";  // Default directory for Windows
#elif defined(Q_OS_LINUX)
    defaultDir = "/home";  // Default directory for Linux
#elif defined(Q_OS_MAC)
    defaultDir = "/Users";  // Default directory for macOS
#else
    defaultDir = "/";  // Fallback to root if OS is unknown
#endif

    // Open a file dialog to select a folder
    // QString selectedDirectory = QFileDialog::getExistingDirectory(
    //     nullptr,                               // Parent widget (null for no parent)
    //     "Select Folder",                       // Dialog title
    //     defaultDir,                               // Default directory (you can set your default path)
    //     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks  // Show only directories
    //     );

    QString selectedDirectory = "/Users/ktt/Downloads/0ATest_input";

    if (!selectedDirectory.isEmpty()) {
        qDebug() << "Selected folder:" << selectedDirectory;
        ascFileList = searchForFilesWithPattern(selectedDirectory,"*.asc");
        dbFileList = searchForFilesWithPattern(selectedDirectory,"*.db");
        ymlFileList = searchForFilesWithPattern(selectedDirectory,"*.yml");
        csvFileList = searchForFilesWithPattern(selectedDirectory,"*.csv");
        int ascFilesCount = ascFileList.size();
        int dbFilesCount = dbFileList.size();
        if(ascFilesCount + dbFilesCount == 0){
            QMessageBox::information(this, "Information", "No .asc or .db files found in the selected folder.");
        }
        else{
            all_rasters_exist = true;
        }
        statusMessage = "Loaded " + selectedDirectory + " (" + QString::number(ascFilesCount) + " .asc files, " + QString::number(dbFilesCount) + " .db files)";
    } else {
        qDebug() << "No folder selected.";
    }
    ui->statusbar->showMessage(statusMessage);

    if(all_rasters_exist){
        enableInputWidgets();
        ui->le_sim_path->setText(selectedDirectory);

        //Display only filenames in the combobox
        QStringList ascFileNameList;
        for(const QString &ascFilePath : ascFileList){
            QFileInfo fileInfo(ascFilePath);
            ascFileNameList.append(fileInfo.fileName());
        }

        QStringListModel *model = new QStringListModel(this);
        model->setStringList(ascFileNameList);
        ui->cb_raster_list->setModel(model);
    }
}


void MainWindow::on_cb_raster_list_activated(int index)
{
    loader = nullptr;
    loader = new LoaderRaster();
    loader->loadFileSingle(ascFileList[index], vizData, nullptr, nullptr);
    ui->openGLWidget->updateInstanceData(vizData, ui->openGLWidget->width(),ui->openGLWidget->height());
    ui->openGLWidget->updateVertexBuffers();
}


void MainWindow::on_cb_raster_list_currentIndexChanged(int index)
{
    loader = nullptr;
    loader = new LoaderRaster();
    loader->loadFileSingle(ascFileList[index], vizData, nullptr, nullptr);
    ui->openGLWidget->updateInstanceData(vizData, ui->openGLWidget->width(),ui->openGLWidget->height());
    ui->openGLWidget->updateVertexBuffers();
}

void MainWindow::on_bt_process_clicked()
{
    disabeInputWidgets();
    loader = nullptr;
    loader = new LoaderYML();
    loader->loadFileSingle(ymlFileList[0], vizData, nullptr, nullptr);

    loader = nullptr;
    loader = new LoaderSQLite();
    loader->loadDBList(dbFileList, "locationid", "monthlydataid", "pfpr2to10,population", "monthlysitedata", vizData,
                       [this](int progress) {  // Progress callback
                            QMetaObject::invokeMethod(this, [this, progress]() {
                                ui->progress_bar->setValue(progress);  // Update the progress bar value
                                ui->statusbar->showMessage("Loading database files... " + QString::number(progress) + "%");
                            }, Qt::QueuedConnection);
                        },
                       [this]() {  // Completion callback
                           QMetaObject::invokeMethod(this, [this]() {
                               qDebug() << "Loading complete!";
                               ui->statusbar->showMessage("Loading database complete!");

                               // Do whatever is needed after loading is complete
                               for (int i = 0; i < vizData->statsData[0].data.size(); i++) {
                                   int lastLoc = vizData->statsData[0].data[i].size();
                                   int lastMonth = vizData->statsData[0].data[i][lastLoc - 1].size();
                                   // qDebug() << "DB: " << dbFileList[i] << " 151 pfpr2to10: " << vizData->statsData[0].data[i][150][150] << " pop: " << vizData->statsData[1].data[i][150][150];
                                   qDebug() << "DB: " << i << " 151 pfpr2to10: " << vizData->statsData[0].data[i][150][150] << " pop: " << vizData->statsData[1].data[i][150][150];
                               }

                               // Dynamically allocate the DataProcessor object
                               dataProcessor = new DataProcessor();

                               dataProcessor->processStatsData(vizData,
                                                               [this](int progress) {  // Progress callback
                                                                   QMetaObject::invokeMethod(this, [this, progress]() {
                                                                       ui->progress_bar->setValue(progress);  // Update the progress bar value
                                                                       ui->statusbar->showMessage("Calculating IQR ... " + QString::number(progress) + "%");
                                                                   }, Qt::QueuedConnection);
                                                               },
                                                               [this]() {  // Completion callback
                                                                   QMetaObject::invokeMethod(this, [this]() {
                                                                       qDebug() << "Calculating IQR complete!";
                                                                       ui->statusbar->showMessage("Calculating IQR complete!");

                                                                       for(int i = 0; i < vizData->statsData.size(); i++){
                                                                           qDebug() << "Month 150 5 150: " << vizData->statsData[i].iqr5[150][150];
                                                                           qDebug() << "Month 150 25 150: " << vizData->statsData[i].iqr25[150][150];
                                                                           qDebug() << "Month 150 Median 150: " << vizData->statsData[i].median[150][150];
                                                                           qDebug() << "Month 150 75 150: " << vizData->statsData[i].iqr75[150][150];
                                                                           qDebug() << "Month 150 95 150: " << vizData->statsData[i].iqr95[150][150];
                                                                       }
                                                                   }, Qt::QueuedConnection);
                                                               });

                               enableInputWidgets();
                           }, Qt::QueuedConnection);
                       });
}


void MainWindow::on_bt_run_clicked()
{
    qDebug() << "Median: " << vizData->statsData[0].median.size() << " x " << vizData->statsData[0].median[0].size();
    qDebug() << "Min: " << vizData->statsData[0].medianMin << " Max: " << vizData->statsData[0].medianMax;

    QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>(this);

    // Define a lambda function to update the data in a separate thread
    QFuture<void> future = QtConcurrent::run([=]() {
        for (int month = 0; month < vizData->statsData[0].median.size(); month++) {
            // Update raster data with the new PfPR values
            ui->openGLWidget->updateInstanceDataMedian(vizData, month);

            // Ensure updateVertexBuffers() is called in the main thread
            QMetaObject::invokeMethod(ui->openGLWidget, "updateVertexBuffers", Qt::QueuedConnection);
            qDebug() << "Month: " << month;

            // Progress bar needs to be updated in the main thread
            QMetaObject::invokeMethod(this, [=]() {
                ui->progress_bar->setValue(month * 100 / vizData->statsData[0].median.size());
            }, Qt::QueuedConnection);

            // Sleep for 50 milliseconds to simulate processing time
            QThread::msleep(50);
        }
    });

    // Set the future to the watcher
    futureWatcher->setFuture(future);

    // Connect the futureWatcher to a slot to handle when the background task finishes
    QObject::connect(futureWatcher, &QFutureWatcher<void>::finished, this, [=]() {
        qDebug() << "Processing complete.";
        // Optionally: enable buttons or other UI elements once processing is done
    });
}

void MainWindow::on_le_sim_path_returnPressed()
{
    statusMessage = "No folder selected.";

    // Open a file dialog to select a folder
    QString selectedDirectory = ui->le_sim_path->text();

    if (!selectedDirectory.isEmpty()) {
        qDebug() << "Selected folder:" << selectedDirectory;
        ascFileList = searchForFilesWithPattern(selectedDirectory,"*.asc");
        dbFileList = searchForFilesWithPattern(selectedDirectory,"*.db");
        ymlFileList = searchForFilesWithPattern(selectedDirectory,"*.yml");
        csvFileList = searchForFilesWithPattern(selectedDirectory,"*.csv");
        int ascFilesCount = ascFileList.size();
        int dbFilesCount = dbFileList.size();
        if(ascFilesCount + dbFilesCount == 0){
            QMessageBox::information(this, "Information", "No .asc or .db files found in the selected folder.");
        }
        else{
            all_rasters_exist = true;
        }
        statusMessage = "Loaded " + selectedDirectory + " (" + QString::number(ascFilesCount) + " .asc files, " + QString::number(dbFilesCount) + " .db files)";
    } else {
        qDebug() << "No folder selected.";
    }
    ui->statusbar->showMessage(statusMessage);

    if(all_rasters_exist){
        enableInputWidgets();
        //Display only filenames in the combobox
        QStringList ascFileNameList;
        for(const QString &ascFilePath : ascFileList){
            QFileInfo fileInfo(ascFilePath);
            ascFileNameList.append(fileInfo.fileName());
        }

        QStringListModel *model = new QStringListModel(this);
        model->setStringList(ascFileNameList);
        ui->cb_raster_list->setModel(model);
    }
}

void MainWindow::disabeInputWidgets(){
    ui->le_sim_path->setEnabled(false);
    ui->bt_process->setEnabled(false);
    ui->cb_raster_list->setEnabled(false);
    ui->bt_auto_load_folder->setEnabled(false);
    ui->bt_run->setEnabled(false);
    ui->openGLWidget->setEnabled(false);
}

void MainWindow::enableInputWidgets(){
    ui->le_sim_path->setEnabled(true);
    ui->bt_process->setEnabled(true);
    ui->cb_raster_list->setEnabled(true);
    ui->bt_auto_load_folder->setEnabled(true);
    ui->progress_bar->setValue(0);
    ui->bt_run->setEnabled(true);
    ui->openGLWidget->setEnabled(true);
}

