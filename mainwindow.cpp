#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QResizeEvent>
#include <QFileDialog>
#include <QDirIterator>
#include <QDebug>
#include <QMessageBox>
#include <QStringListModel>
#include <QGraphicsEllipseItem>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFutureWatcher>
#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QDebug>
#include <QLabel>
#include <QList>
#include <QStandardItemModel>

#include "loadersqlite.h"
#include "loaderyml.h"
#include "loaderraster.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setSceneCustom(scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::TextAntialiasing);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

    vizData = new VizData();
    ui->graphicsView->setVizData(vizData);

    ui->le_sim_path->setText("Input simulation path then [Enter] or using [Browse] button");
    stopLoop = false;

    chart = new ChartCustom(this);
    chart->setChartView(ui->gv_chartview);
    chart->setVizData(vizData);

    currentColNameShown = "";
    currentMonth = 0;
    currentLocationSelectedMap = QMap<int,QColor>();

    ui->wg_color_map->setHidden(true);

    QObject::connect(ui->graphicsView, &GraphicsViewCustom::squareClickedOnScene, this, &MainWindow::onSquareClicked);
    QObject::connect(this, &MainWindow::addClearButton, ui->graphicsView, &GraphicsViewCustom::showClearButton);


    hideMedianItems();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
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

// Function to display sqlData in a dialog with checkable columns in a checklist per tab
// Function to display sqlData in a dialog with checkable columns in a checklist per tab
bool displaySqlDataInDialogWithChecklist(VizData* vizData, QWidget* parentWidget = nullptr) {
    // Create the dialog
    QDialog* dialog = new QDialog(parentWidget);
    dialog->setWindowTitle("Select columns to plot");
    dialog->resize(600, 400); // Set an initial size for the dialog
    dialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    // Create a QTabWidget to display the tables
    QTabWidget* tabWidget = new QTabWidget(dialog);

    QMap<QString, QList<bool>> checkBoxStatus;
    QList<QTableWidget*> tableWidgets; // Store the table widgets for each tab

    // Preload the state for each table
    for (int dbIndex = 0; dbIndex < vizData->sqlData.dbPaths.size(); ++dbIndex) {
        QStringList tables = vizData->sqlData.dbTables;
        checkBoxStatus = QMap<QString, QList<bool>>();
        for (const QString& tableName : tables) {
            int tableIndex = vizData->sqlData.dbTables.indexOf(tableName);
            int columnCount = vizData->sqlData.dbColumns[tableIndex].size();
            if (columnCount == 0) {
                qDebug() << "No columns found for table:" << tableName;
                continue;
            }

            // Initialize the checkbox status (all unchecked initially)
            checkBoxStatus[tableName] = QList<bool>(columnCount, false);

            // Create a new QWidget for each tab and its layout
            QWidget* tabContent = new QWidget();
            QVBoxLayout* tabLayout = new QVBoxLayout(tabContent);

            // Create a table widget to hold the checkboxes and column names
            QTableWidget* tableWidget = new QTableWidget();
            tableWidget->setColumnCount(2); // Two columns: one for the checkbox, one for the column name
            tableWidget->setHorizontalHeaderLabels(QStringList() << "Select?" << "Column Names");
            tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Stretch columns to fit
            tableWidget->setRowCount(columnCount); // Set the number of rows for the current table
            tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

            tableWidgets.append(tableWidget); // Store table widget reference for later use

            // Populate the tableWidget with checkboxes and column names
            for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
                QString columnName = vizData->sqlData.dbColumns[tableIndex][columnIndex];

                // Create a checkbox item
                QCheckBox *checkBox = new QCheckBox();
                tableWidget->setCellWidget(columnIndex, 0, checkBox); // Add the checkbox to the first column
                checkBox->setChecked(checkBoxStatus[tableName][columnIndex]);  // Set the checkbox state
                // Capture the checkbox state when it is changed
                QObject::connect(checkBox, &QCheckBox::stateChanged, [tableName, columnIndex, &checkBoxStatus](int state) {
                    checkBoxStatus[tableName][columnIndex] = (state == Qt::Checked);  // Update the checkbox status
                });

                // Create an item for the column name
                QTableWidgetItem* item = new QTableWidgetItem(columnName);
                tableWidget->setItem(columnIndex, 1, item);
            }

            // Add the table widget to the tab layout
            tabLayout->addWidget(tableWidget);
            tabContent->setLayout(tabLayout);

            // Add the tab to the QTabWidget
            tabWidget->addTab(tabContent, tableName);
        }
    }

    // Create QLineEdit fields for locationID and monthID
    QLineEdit* locationIdEdit = new QLineEdit(dialog);
    locationIdEdit->setPlaceholderText(vizData->sqlData.locationID + " (default)");

    QLineEdit* monthIdEdit = new QLineEdit(dialog);
    monthIdEdit->setPlaceholderText(vizData->sqlData.monthID + " (default)");

    // Create labels for the LocationID and MonthID fields
    QLabel* locationIdLabel = new QLabel("Location column name:", dialog);
    QLabel* monthIdLabel = new QLabel("Month column name:", dialog);

    // Create layouts to hold the label and text field side by side
    QHBoxLayout* locationIdLayout = new QHBoxLayout();
    locationIdLayout->addWidget(locationIdLabel);
    locationIdLayout->addWidget(locationIdEdit);

    QHBoxLayout* monthIdLayout = new QHBoxLayout();
    monthIdLayout->addWidget(monthIdLabel);
    monthIdLayout->addWidget(monthIdEdit);

    // Create a QDialogButtonBox with OK and Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect the OK button to dialog's accept slot and Cancel button to reject slot
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // Create a layout for the dialog
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(tabWidget);      // Add the QTabWidget to the layout
    layout->addLayout(locationIdLayout); // Add the location ID label and text field
    layout->addLayout(monthIdLayout);    // Add the month ID label and text field
    layout->addWidget(buttonBox);      // Add the button box to the layout

    // Set the layout for the dialog
    dialog->setLayout(layout);


    // Set the layout for the dialog
    dialog->setLayout(layout);

    // Slot to reset all checkboxes in the current tab
    QObject::connect(tabWidget, &QTabWidget::currentChanged, [=](int index) {
        if (index >= 0 && index < tableWidgets.size()) {
            QTableWidget* currentTable = tableWidgets[index];
            for (int i = 0; i < currentTable->rowCount(); ++i) {
                QWidget* widget = currentTable->cellWidget(i, 0);
                if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
                    checkBox->setChecked(false); // Reset all checkboxes
                }
            }
        }
    });

    // Show the dialog
    dialog->exec();

    // After the dialog is accepted, you can access the selected columns for each table like this:
    if (dialog->result() == QDialog::Accepted) {
        vizData->sqlData.tableColumnsMap.clear(); // Clear the previous column map
        qDebug() << "Dialog accepted";  // Add this for debugging
        QString locationID = locationIdEdit->text();  // Capture locationID
        QString monthID = monthIdEdit->text();        // Capture monthID
        qDebug() << "Location :" << locationID;
        qDebug() << "Month ID:" << monthID;

        for (const QString& tableName : checkBoxStatus.keys()) {
            int tableIndex = vizData->sqlData.dbTables.indexOf(tableName);
            QString selectedColumns = "";
            for (int j = 0; j < checkBoxStatus[tableName].size(); ++j) {
                if (checkBoxStatus[tableName][j]) {
                    selectedColumns += vizData->sqlData.dbColumns[tableIndex][j] + ",";
                }
            }
            if (!selectedColumns.isEmpty()) {
                selectedColumns.chop(1); // Remove the last comma
                vizData->sqlData.tableColumnsMap[tableName] = selectedColumns;
            }
        }
        for (const QString& tableName : vizData->sqlData.tableColumnsMap.keys()) {
            qDebug() << "Table:" << tableName << " Columns:" << vizData->sqlData.tableColumnsMap[tableName];
        }
        return true;
    } else {
        vizData->sqlData.tableColumnsMap.clear();
        qDebug() << "Dialog rejected";  // Add this for debugging if Cancel is pressed or dialog is closed
        return false;
    }
}


void MainWindow::on_cb_raster_list_currentIndexChanged(int index)
{
    LoaderRaster *loader = new LoaderRaster();
    loader->loadFileSingle(ascFileList[index], vizData, nullptr, nullptr);

    qDebug() << "ncols:" << vizData->rasterData->raster->NCOLS << " nrows:" << vizData->rasterData->raster->NROWS;

    ui->wg_color_map->setColorMapMinMax(QPair<double,double>(vizData->rasterData->dataMin, vizData->rasterData->dataMax));

    ui->graphicsView->updateRasterData();
}

void MainWindow::onSquareClicked(const QPoint &pos, const QColor &color)
{
    int location = vizData->rasterData->locationPair2DTo1D[QPair<int,int>(pos.y(),pos.x())];
    if(currentLocationSelectedMap.contains(location)){
        currentLocationSelectedMap.remove(location);
    }
    else{
        if(pos.x() == -1 && pos.y() == -1){
            currentLocationSelectedMap.clear();
            qDebug() << "[Main]Clear all locations";
        }
        else{
            currentLocationSelectedMap[location] = color;
        }
    }

    qDebug() << "[Main]Square select at:" << pos << "loc: " << location << "color: " << color;
    if(ui->cb_col_name_list->isVisible()){
        showChart();
    }
    else{
        if(currentLocationSelectedMap.isEmpty()){
            ui->statusbar->showMessage("No location selected");
        }
        else{
            double data = vizData->rasterData->raster->data[pos.y()][pos.x()];
            ui->statusbar->showMessage("Location: " + QString::number(location)
                                       + "(row: " + QString::number(pos.y()) + ", col: " + QString::number(pos.x()) + ")"
                                       + " Value: " + QString::number(data));
        }
    }

    emit(addClearButton(!currentLocationSelectedMap.empty()));
}

void MainWindow::onMouseMoved(const QPoint &pos)
{
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

    QString selectedDirectory = QFileDialog::getExistingDirectory(
        nullptr,                               // Parent widget (null for no parent)
        "Select Folder",                       // Dialog title
        defaultDir,                               // Default directory (you can set your default path)
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks  // Show only directories
        );

    // QString selectedDirectory = "/Users/ktt/Downloads/0ATest_input";

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
        resetMedianMap();
        enableInputWidgets();
        showItemsAfterBrowseClicked();

        ui->le_sim_path->setText(selectedDirectory);
        vizData->currentDirectory = selectedDirectory;

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

void MainWindow::on_bt_process_clicked()
{
    loader = nullptr;
    loader = new LoaderYML();
    loader->loadFileSingle(ymlFileList[0], vizData, nullptr, nullptr);

    loader = nullptr;
    loader = new LoaderSQLite();

    loader->loadFileSingle(dbFileList[0], vizData, nullptr, nullptr);

    if(!displaySqlDataInDialogWithChecklist(vizData, this)){
        return;
    }      

    ui->statusbar->showMessage("Loading database files...");

    if(vizData->sqlData.tableColumnsMap.isEmpty()){
        QMessageBox::information(this, "Information", "No columns selected.");
        return;
    }

    disabeInputWidgets();

    loader->loadDBList(dbFileList,
                       vizData->sqlData.locationID,
                       vizData->sqlData.monthID,
                       vizData->sqlData.tableColumnsMap[vizData->sqlData.tableColumnsMap.keys().last()],
                       vizData->sqlData.tableColumnsMap.keys().last(), vizData,
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

                               // for(QString colName : vizData->statsData.keys()){
                               //     for (int j = 0; j < vizData->statsData[colName].data.size(); j++) {
                               //         qDebug() << "Data:" << colName << "j:" << vizData->statsData[colName].data[j][150][150];
                               //     }
                               // }

                               QString tableName = vizData->sqlData.tableColumnsMap.keys().last();
                               int tableIndex = vizData->sqlData.dbTables.indexOf(tableName);
                               QFile file(QDir(vizData->currentDirectory).filePath("MaSimViz_"+vizData->sqlData.tableColumnsMap.keys().last() + ".dat"));
                               if(file.exists()){
                                   loadStatsData(tableName);
                               }
                               else{
                                   processAndSaveStatsData();
                               }
                           }, Qt::QueuedConnection);
                       });
}

void MainWindow::on_bt_run_clicked()
{
    // Otherwise, start or resume the loop
    if(showItemsAfterRunClicked())
        return;

    QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>(this);

    // Define a lambda function to update the data in a separate thread
    QFuture<void> future = QtConcurrent::run([=]() {
        for (int month = currentMonth; month < vizData->statsData[currentColNameShown].iqr[0].size(); month++) {
            // Check if the loop should be stopped
            if (stopLoop) {
                currentMonth = month;
                break;
            }

            // Progress bar needs to be updated in the main thread
            QMetaObject::invokeMethod(this, [=]() {
                ui->slider_progress->setValue(month);
            }, Qt::QueuedConnection);

            // Sleep for 50 milliseconds to simulate processing time
            QThread::msleep(50);
        }
    });

    // Set the future to the watcher
    futureWatcher->setFuture(future);

    // Connect the futureWatcher to a slot to handle when the background task finishes
    QObject::connect(futureWatcher, &QFutureWatcher<void>::finished, this, [=]() {
        // Reset the state when processing finishes
        qDebug() << "Running finished!";
        if (!stopLoop)
        {
            qDebug() << "Running finished !stopLoop !";
            resetPlayState();
        }
        isRunning = false;
        stopLoop = false;
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

void MainWindow::on_slider_progress_valueChanged(int value)
{
    currentMonth = value + 1;
    if (currentMonth >= vizData->monthCountStartToEnd) {
        currentMonth = vizData->monthCountStartToEnd - 1;
    }
    showMedianMap();
    showChart();
    ui->statusbar->showMessage("Month: " + QString::number(currentMonth) + " Year: " + QString::number(currentMonth / 12));
    // qDebug() << value << "Month:" << currentMonth;
}


void MainWindow::on_slider_progress_sliderMoved(int position)
{
    isRunning = false;
    stopLoop = true;
}


void MainWindow::on_cb_col_name_list_currentTextChanged(const QString &colName)
{
    qDebug() << "Column name changed to:" << colName;
    currentColNameShown = colName;
    showMedianMap();
    showChart();
}

void MainWindow::disabeInputWidgets(){
    ui->le_sim_path->setEnabled(false);
    ui->bt_process->setEnabled(false);
    ui->cb_raster_list->setEnabled(false);
    ui->cb_col_name_list->setEnabled(false);
    ui->bt_auto_load_folder->setEnabled(false);
    ui->bt_run->setEnabled(false);
    ui->graphicsView->setEnabled(false);
    ui->slider_progress->setEnabled(false);
}

void MainWindow::enableInputWidgets(){
    ui->le_sim_path->setEnabled(true);
    ui->bt_process->setEnabled(true);
    ui->cb_raster_list->setEnabled(true);
    ui->cb_col_name_list->setEnabled(true);
    ui->bt_auto_load_folder->setEnabled(true);
    ui->progress_bar->setValue(0);
    ui->bt_run->setEnabled(true);
    ui->graphicsView->setEnabled(true);
    ui->slider_progress->setEnabled(true);
}

void MainWindow::resetMedianMap(){
    if(vizData->statsData.isEmpty()){
        currentColNameShown = "";
    }
    else{
        currentColNameShown = vizData->statsData.keys().first();
    }
    currentMonth = 0;
    currentLocationSelectedMap.clear();
    ui->graphicsView->resetGraphicsView();
    ui->gv_chartview->setHidden(currentLocationSelectedMap.isEmpty());
}

void MainWindow::showMedianMap(){
    ui->wg_color_map->setColorMapMinMax(QPair<double,double>(vizData->statsData[currentColNameShown].medianMin, vizData->statsData[currentColNameShown].medianMax));
    ui->graphicsView->updateRasterDataMedian(currentColNameShown, currentMonth);
    ui->graphicsView->update();
}

void MainWindow::showChart(){
    ui->gv_chartview->setHidden(currentLocationSelectedMap.isEmpty());
    if(ui->gv_chartview->isVisible()){
        chart->plotDataMedianMultipleLocations(currentColNameShown, currentLocationSelectedMap, currentMonth, ui->cb_col_name_list->currentText());
    }
}

void MainWindow::hideMedianItems(){
    ui->slider_progress->setHidden(true);
    ui->cb_col_name_list->setHidden(true);
    ui->gv_chartview->setHidden(true);
}

void MainWindow::showItemsAfterBrowseClicked(){
    ui->cb_raster_list->setHidden(false);
    ui->cb_col_name_list->setHidden(ui->cb_raster_list->isVisible());
    ui->slider_progress->setHidden(true);
    ui->progress_bar->setHidden(ui->slider_progress->isVisible());
    ui->slider_progress->setValue(0);
    ui->progress_bar->setValue(0);
    ui->bt_run->setEnabled(false);
    ui->wg_color_map->setHidden(false);
    ui->gv_chartview->setHidden(true);
}

void MainWindow::showItemsAfterProcessClicked(){
    ui->cb_col_name_list->setHidden(false);
    ui->cb_raster_list->setHidden(ui->cb_col_name_list->isVisible());
    ui->slider_progress->setHidden(false);
    ui->progress_bar->setHidden(ui->slider_progress->isVisible());
    ui->slider_progress->setValue(0);
    ui->slider_progress->setMaximum(vizData->monthCountStartToEnd - 1);
    ui->progress_bar->setValue(0);
    isRunning = false;
    stopLoop = false;
}

bool MainWindow::showItemsAfterRunClicked(){
    if (isRunning) {
        // If running, pause the loop
        stopLoop = true;
        isRunning = false;
        ui->bt_run->setText("Run");
        return true;
    }
    ui->bt_run->setText("Pause");
    isRunning = true;
    stopLoop = false;
    if(ui->progress_bar->isVisible()){
        ui->progress_bar->setHidden(true);
    }
    ui->slider_progress->setHidden(ui->progress_bar->isVisible());
    return false;
}

void MainWindow::resetPlayState(){
    isRunning = false;
    stopLoop = false;
    ui->bt_run->setText("Run");
    ui->progress_bar->setValue(0);
    ui->slider_progress->setValue(0);
    ui->statusbar->showMessage("Playing complete.");
}

void MainWindow::saveStatsData(){
    dataProcessor->saveStatsDataToCSV(vizData,
                                      [this](int progress) {  // Progress callback
                                          QMetaObject::invokeMethod(this, [this, progress]() {
                                              ui->progress_bar->setValue(progress);  // Update the progress bar value
                                              ui->statusbar->showMessage("Saving IQR data to CSV ... " + QString::number(progress) + "%");
                                          }, Qt::QueuedConnection);
                                      },
                                      [this]() {  // Completion callback
                                          QMetaObject::invokeMethod(this, [this]() {
                                              qDebug() << "Saving IQR data to CSV complete!";
                                              ui->statusbar->showMessage("Saving IQR data to CSV complete!");

                                              QStringListModel *model = new QStringListModel(this);
                                              QStringList columnList = vizData->statsData.keys();
                                              model->setStringList(columnList);
                                              ui->cb_col_name_list->setModel(model);

                                              resetMedianMap();
                                              showItemsAfterProcessClicked();
                                              showMedianMap();
                                              enableInputWidgets();
                                          }, Qt::QueuedConnection);
                                      });
}

void MainWindow::processAndSaveStatsData(){
    dataProcessor->processStatsData(vizData,[this](int progress) {  // Progress callback
                                                QMetaObject::invokeMethod(this, [this, progress]() {
                                                    ui->progress_bar->setValue(progress);  // Update the progress bar value
                                                    ui->statusbar->showMessage("Calculating IQR ... " + QString::number(progress) + "%");
                                                }, Qt::QueuedConnection);
                                            },
                                            [this]() {  // Completion callback
                                                QMetaObject::invokeMethod(this, [this]() {
                                                    qDebug() << "Calculating IQR complete!";
                                                    ui->statusbar->showMessage("Calculating IQR complete!");

                                                    // for(QString colName : vizData->statsData.keys()){
                                                    //     qDebug() << "[Save]" << colName << vizData->statsData[colName].iqr[0].size() << "x" << vizData->statsData[colName].iqr[0][0].size();
                                                    //     qDebug() << "[Save]Month 150 5 loc 150: " << vizData->statsData[colName].iqr[1][150][150];
                                                    //     qDebug() << "[Save]Month 150 25 loc 150: " << vizData->statsData[colName].iqr[2][150][150];
                                                    //     qDebug() << "[Save]Month 150 Median loc 150: " << vizData->statsData[colName].iqr[0][150][150];
                                                    //     qDebug() << "[Save]Month 150 75 loc 150: " << vizData->statsData[colName].iqr[3][150][150];
                                                    //     qDebug() << "[Save]Month 150 95 loc 150: " << vizData->statsData[colName].iqr[4][150][150];
                                                    //     qDebug() << "[Save]Month 150 min loc 150: " << vizData->statsData[colName].medianMin;
                                                    //     qDebug() << "[Save]Month 150 max loc 150: " << vizData->statsData[colName].medianMax;
                                                    // }
                                                    saveStatsData();
                                        }, Qt::QueuedConnection);
                                    });
}

void MainWindow::loadStatsData(QString tableName){
    dataProcessor->loadStatsDataFromCSV(tableName, vizData,
                                        [this](int progress) {  // Progress callback
                                            QMetaObject::invokeMethod(this, [this, progress]() {
                                                ui->progress_bar->setValue(progress);  // Update the progress bar value
                                                ui->statusbar->showMessage("Loading IQR data from CSV ... " + QString::number(progress) + "%");
                                            }, Qt::QueuedConnection);
                                        },
                                        [this](int readCode) {  // Completion callback
                                            QMetaObject::invokeMethod(this, [this, readCode]() {
                                                qDebug() << "Loading IQR data from CSV complete!";
                                                ui->statusbar->showMessage("Loading IQR data from CSV complete!");

                                                switch (readCode) {
                                                case 0:{
                                                    qDebug() << "File loaded successfully!";
                                                    break;
                                                }
                                                case 1:
                                                    QMessageBox::information(this, "Information", "Unable to open file for reading");
                                                    break;
                                                case 2:
                                                    QMessageBox::information(this, "Information", "Column number does not match");
                                                    break;
                                                case 3:
                                                    QMessageBox::information(this, "Information", "Header size does not match config location and columns");
                                                    break;
                                                case 4:
                                                    QMessageBox::information(this, "Information", "Column names do not match selected columns");
                                                    break;
                                                }
                                                if(readCode == 0){
                                                    QStringListModel *model = new QStringListModel(this);
                                                    QStringList columnList = vizData->statsData.keys();
                                                    model->setStringList(columnList);
                                                    ui->cb_col_name_list->setModel(model);

                                                    // for(QString colName : vizData->statsData.keys()){
                                                    //     qDebug() << "[Load]" << colName << vizData->statsData[colName].iqr[0].size() << "x" << vizData->statsData[colName].iqr[0][0].size();
                                                    //     qDebug() << "[Load]Month 150 5 loc 150: " << vizData->statsData[colName].iqr[1][150][150];
                                                    //     qDebug() << "[Load]Month 150 25 loc 150: " << vizData->statsData[colName].iqr[2][150][150];
                                                    //     qDebug() << "[Load]Month 150 Median loc 150: " << vizData->statsData[colName].iqr[0][150][150];
                                                    //     qDebug() << "[Load]Month 150 75 loc 150: " << vizData->statsData[colName].iqr[3][150][150];
                                                    //     qDebug() << "[Load]Month 150 95 loc 150: " << vizData->statsData[colName].iqr[4][150][150];
                                                    //     qDebug() << "[Load]Month 150 min loc 150: " << vizData->statsData[colName].medianMin;
                                                    //     qDebug() << "[Load]Month 150 max loc 150: " << vizData->statsData[colName].medianMax;
                                                    // }

                                                    resetMedianMap();
                                                    showItemsAfterProcessClicked();
                                                    showMedianMap();
                                                    enableInputWidgets();
                                                }
                                                else {
                                                    //Create a dialog and ask if user want to generate new stats
                                                    QMessageBox msgBox;
                                                    msgBox.setText("Do you want to generate new data?");
                                                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                                                    msgBox.setDefaultButton(QMessageBox::No);
                                                    int ret = msgBox.exec();
                                                    if(ret == QMessageBox::Yes){
                                                        processAndSaveStatsData();
                                                    }
                                                    else{
                                                        enableInputWidgets();
                                                    }
                                                    return;
                                                }
                                            }, Qt::QueuedConnection);
                                        });
}
