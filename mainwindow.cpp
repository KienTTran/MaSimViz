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
    vizData = new VizData();

    ui->le_sim_path->setText("/Users/ktt/Downloads/0ATest_input");
    ui->slider_progress->setHidden(true);
    ui->cb_db_list->setHidden(true);
    stopLoop = false;
    ui->cb_show_chart->setHidden(true);
    ui->gv_chartview->setHidden(true);
    ui->tableView->setHidden(true);
    QObject::connect(ui->openGLWidget, &GLWidgetCustom::mouseMoved, this, &MainWindow::onMouseMoved);
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
bool displaySqlDataInDialogWithChecklist(VizData* vizData, QWidget* parentWidget = nullptr) {
    // Create the dialog
    QDialog* dialog = new QDialog(parentWidget);
    dialog->setWindowTitle("SQL Data Viewer with Checklists");
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

            tableWidgets.append(tableWidget); // Store table widget reference for later use

            // Populate the tableWidget with checkboxes and column names
            for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
                QString columnName = vizData->sqlData.dbColumns[tableIndex][columnIndex];

                // Disable column locationID and monthID checkbox
                if (columnName != vizData->sqlData.locationID && columnName != vizData->sqlData.monthID) {
                    // Create a checkbox item
                    QCheckBox *checkBox = new QCheckBox();
                    tableWidget->setCellWidget(columnIndex, 0, checkBox); // Add the checkbox to the first column
                    checkBox->setChecked(checkBoxStatus[tableName][columnIndex]);  // Set the checkbox state
                    // Capture the checkbox state when it is changed
                    QObject::connect(checkBox, &QCheckBox::stateChanged, [tableName, columnIndex, &checkBoxStatus](int state) {
                        checkBoxStatus[tableName][columnIndex] = (state == Qt::Checked);  // Update the checkbox status
                    });
                }

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

    // Create a QDialogButtonBox with OK and Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect the OK button to dialog's accept slot and Cancel button to reject slot
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // Create a layout for the dialog
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(tabWidget);   // Add the QTabWidget to the layout
    layout->addWidget(buttonBox);   // Add the button box to the layout

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
        vizData->sqlData.tableColumnMap.clear(); // Clear the previous column map
        qDebug() << "Dialog accepted";  // Add this for debugging
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
                vizData->sqlData.tableColumnMap[tableName] = selectedColumns;
            }
        }
        for (const QString& tableName : vizData->sqlData.tableColumnMap.keys()) {
            qDebug() << "Table:" << tableName << " Columns:" << vizData->sqlData.tableColumnMap[tableName];
        }
        return true;
    } else {
        vizData->sqlData.tableColumnMap.clear();
        qDebug() << "Dialog rejected";  // Add this for debugging if Cancel is pressed or dialog is closed
        return false;
    }
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

        ui->progress_bar->setHidden(false);
        ui->slider_progress->setHidden(true);

        ui->cb_show_chart->setCheckState(Qt::Unchecked);
        ui->cb_show_chart->setChecked(false);
        ui->cb_show_chart->setHidden(true);

        ui->cb_db_list->setHidden(true);
        ui->cb_raster_list->setHidden(false);

        ui->bt_run->setText("Run");
        ui->progress_bar->setValue(0);
        ui->slider_progress->setValue(0);
        isRunning = false;
        stopLoop = false;

        ui->bt_run->setEnabled(false);
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
    // qDebug() << "Activated:" << index;

    // if(!vizData->rasterData->values.isEmpty()){
    //     loader = nullptr;
    //     loader = new LoaderRaster();
    //     loader->loadFileSingle(ascFileList[index], vizData, nullptr, nullptr);
    //     ui->openGLWidget->vizData = vizData;
    //     ui->openGLWidget->updateInstanceData(ui->openGLWidget->width(),ui->openGLWidget->height());
    //     ui->openGLWidget->updateVertexBuffers();
    //     ui->openGLWidget->updateVertexBuffers();

    //     qDebug() << "ncols:" << vizData->rasterData->ncols << " nrows:" << vizData->rasterData->nrows;

    //     QStandardItemModel *model = new QStandardItemModel(vizData->rasterData->ncols, vizData->rasterData->nrows, this);

    //     for(int i = 0; i < vizData->rasterData->ncols; i++){
    //         for(int j = 0; j < vizData->rasterData->nrows; j++){
    //             model->setItem(i,j,new QStandardItem(QString::number(vizData->rasterData->values[i][j])));
    //             ui->tableView_2->setRowHeight(j,10);
    //         }
    //         ui->tableView_2->setColumnWidth(i,10);
    //     }
    //     ui->tableView_2->setModel(model);
    // }
    // else{
    //     QMessageBox::information(this, "Information", "No raster data loaded.");
    // }
}


void MainWindow::on_cb_raster_list_currentIndexChanged(int index)
{
    LoaderRaster *loader = new LoaderRaster();
    loader->loadFileSingle(ascFileList[index], vizData, nullptr, nullptr);
    ui->openGLWidget->vizData = vizData;
    ui->openGLWidget->updateInstanceData(ui->openGLWidget->width(),ui->openGLWidget->height());
    ui->openGLWidget->updateVertexData();
    ui->openGLWidget->updateVertexBuffers();
}

void MainWindow::onMouseMoved(const QPoint &pos)
{
    // Convert the mouse position into the index of the data in vizData->statsData.median
    // int dataX = pos.x() / ui->openGLWidget->width() * vizData->statsData[currentColIndexPlaying].median.size(); // Assuming mouse maps to data grid
    // int dataY = pos.y() / ui->openGLWidget->height() * vizData->statsData[currentColIndexPlaying].median[0].size();

    // Display the data in the table
    // displayDataInTable(dataX, dataY);

    displayDataInTable(pos.x(),pos.y());
}


void MainWindow::displayDataInTable(int col, int row)
{
    qDebug() << "Displaying data at:" << col << row;
    ui->statusbar->showMessage(statusMessage + " Displaying data at:" + QString::number(col) + " " + QString::number(row));
    // // 3x3 table view, get the 9 cells around the (col, row) from vizData->statsData.median
    // QStandardItemModel *model = new QStandardItemModel(3, 3, this); // 3x3 table model

    // for (int i = -1; i <= 1; ++i) {
    //     for (int j = -1; j <= 1; ++j) {
    //         int currentRow = row + i;
    //         int currentCol = col + j;

    //         // Ensure we don't go out of bounds
    //         if (currentRow >= 0 && currentRow < vizData->statsData[currentColIndexPlaying].median.size() &&
    //             currentCol >= 0 && currentCol < vizData->statsData[currentColIndexPlaying].median[0].size()) {

    //             double value = vizData->statsData[0].median[currentRow][currentCol];
    //             model->setItem(i + 1, j + 1, new QStandardItem(QString::number(value)));
    //         } else {
    //             model->setItem(i + 1, j + 1, new QStandardItem("N/A"));
    //         }
    //     }
    // }
    // ui->tableView->setModel(model);
}

void MainWindow::on_bt_process_clicked()
{
    loader = nullptr;
    loader = new LoaderYML();
    loader->loadFileSingle(ymlFileList[0], vizData, nullptr, nullptr);

    loader = nullptr;
    loader = new LoaderSQLite();

    loader->loadFileSingle(dbFileList[0], vizData, nullptr, nullptr);

    vizData->sqlData.locationID = "locationid";
    vizData->sqlData.monthID = "monthlydataid";

    if(!displaySqlDataInDialogWithChecklist(vizData, this)){
        return;
    }

    disabeInputWidgets();
    ui->progress_bar->setHidden(false);
    ui->slider_progress->setHidden(true);

    ui->cb_show_chart->setCheckState(Qt::Unchecked);
    ui->cb_show_chart->setChecked(false);
    ui->cb_show_chart->setHidden(true);

    ui->bt_run->setText("Run");
    ui->progress_bar->setValue(0);
    ui->slider_progress->setValue(0);
    isRunning = false;
    stopLoop = false;
    ui->statusbar->showMessage("Loading database files...");

    if(vizData->sqlData.tableColumnMap.isEmpty()){
        QMessageBox::information(this, "Information", "No columns selected.");
        enableInputWidgets();
        return;
    }

    loader->loadDBList(dbFileList,
                       vizData->sqlData.locationID,
                       vizData->sqlData.monthID,
                       vizData->sqlData.tableColumnMap[vizData->sqlData.tableColumnMap.keys().last()],
                       vizData->sqlData.tableColumnMap.keys().last(), vizData,
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
                               for(int i = 0; i < vizData->statsData.size(); i++){
                                   for (int j = 0; j < vizData->statsData[i].data.size(); j++) {
                                       qDebug() << "Data:" << i << "j:" << vizData->statsData[i].data[j][150][150];
                                   }
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

                                                                       QStringListModel *model = new QStringListModel(this);
                                                                       //Here load the only table because 1 table is allowed in this branch
                                                                       QStringList tableList = vizData->sqlData.tableColumnMap[vizData->sqlData.tableColumnMap.keys().last()].split(',');
                                                                       model->setStringList(tableList);
                                                                       currentColIndexPlaying = 0;
                                                                       ui->cb_db_list->setModel(model);

                                                                       ui->cb_show_chart->setHidden(false);
                                                                       QObject::connect(ui->cb_show_chart, &QCheckBox::stateChanged, this, [=](int state) {
                                                                           plotChart(currentColIndexPlaying);
                                                                           ui->gv_chartview->setHidden(state == Qt::Unchecked);
                                                                           ui->tableView->setHidden(state == Qt::Unchecked);
                                                                           if(state == Qt::Checked){
                                                                               ui->openGLWidget->inspectMode = true;
                                                                               double yValue = plotVerticalLineOnChart(currentMonth);
                                                                               ui->statusbar->showMessage("Month: " + QString::number(currentMonth) + " Year: " + QString::number(currentMonth / 12) + " Value: " + QString::number(yValue));

                                                                           }else{
                                                                               ui->openGLWidget->inspectMode = false;
                                                                           }
                                                                           ui->openGLWidget->update();
                                                                       });

                                                                       showWhenPlay();
                                                                       enableInputWidgets();
                                                                   }, Qt::QueuedConnection);
                                                               });
                           }, Qt::QueuedConnection);
                       });
}

void MainWindow::on_bt_run_clicked()
{
    if (isRunning) {
        // If running, pause the loop
        stopLoop = true;
        isRunning = false;
        ui->bt_run->setText("Run");
        qDebug() << "Paused at month:" << currentMonth;
        return;
    }

    // Otherwise, start or resume the loop
    ui->bt_run->setText("Pause");
    showWhenPlay();
    isRunning = true;
    stopLoop = false;

    QObject::connect(ui->slider_progress, &QSlider::valueChanged, this, [=](int value) {
        currentMonth = value * vizData->statsData[currentColIndexPlaying].median.size() / 100;
        if (currentMonth >= vizData->statsData[currentColIndexPlaying].median.size()) {
            currentMonth = vizData->statsData[currentColIndexPlaying].median.size() - 1;
        }
        ui->openGLWidget->updateInstanceDataMedian(currentColIndexPlaying, currentMonth);
        QMetaObject::invokeMethod(ui->openGLWidget, "updateVertexBuffers", Qt::QueuedConnection);
        double yValue = plotVerticalLineOnChart(currentMonth);
        ui->statusbar->showMessage("Month: " + QString::number(currentMonth) + " Year: " + QString::number(currentMonth / 12) + " Value: " + QString::number(yValue));
    });

    QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>(this);

    // Define a lambda function to update the data in a separate thread
    QFuture<void> future = QtConcurrent::run([=]() {
        for (int month = currentMonth; month < vizData->statsData[currentColIndexPlaying].median.size(); month++) {
            // Check if the loop should be stopped
            if (stopLoop) {
                currentMonth = month;  // Save the current month to resume from
                qDebug() << "Stopped at month:" << currentMonth;
                break;
            }

            // Update raster data with the new PfPR values
            ui->openGLWidget->updateInstanceDataMedian(currentColIndexPlaying, month);

            // Ensure updateVertexBuffers() is called in the main thread
            QMetaObject::invokeMethod(ui->openGLWidget, "updateVertexBuffers", Qt::QueuedConnection);

            // Progress bar needs to be updated in the main thread
            QMetaObject::invokeMethod(this, [=]() {
                ui->slider_progress->setValue(month * 100 / vizData->statsData[currentColIndexPlaying].median.size());
                double yValue = plotVerticalLineOnChart(currentMonth);
                ui->statusbar->showMessage("Month: " + QString::number(currentMonth) + " Year: " + QString::number(currentMonth / 12) + " Value: " + QString::number(yValue));
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
        if (!stopLoop)
        {
            currentMonth = 0;  // Reset the month after completion
            ui->slider_progress->setValue(0);
            ui->bt_run->setText("Run");
            ui->statusbar->showMessage("Playing complete.");
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

void MainWindow::showWhenPlay(){
    ui->cb_db_list->setHidden(false);
    ui->cb_raster_list->setHidden(true);
    ui->progress_bar->setHidden(true);
    ui->slider_progress->setHidden(false);
}

void MainWindow::showWhenPause(){
    ui->cb_db_list->setHidden(true);
    ui->cb_raster_list->setHidden(false);
    ui->progress_bar->setHidden(false);
    ui->slider_progress->setHidden(true);
}


void MainWindow::on_slider_progress_valueChanged(int value)
{
    currentMonth = value * vizData->statsData[currentColIndexPlaying].median.size() / 100;
    if (currentMonth >= vizData->statsData[currentColIndexPlaying].median.size()) {
        currentMonth = vizData->statsData[currentColIndexPlaying].median.size() - 1;
    }
    ui->openGLWidget->updateInstanceDataMedian(currentColIndexPlaying, currentMonth);
    QMetaObject::invokeMethod(ui->openGLWidget, "updateVertexBuffers", Qt::QueuedConnection);
    double yValue = plotVerticalLineOnChart(currentMonth);
    ui->statusbar->showMessage("Month: " + QString::number(currentMonth) + " Year: " + QString::number(currentMonth / 12) + " Value: " + QString::number(yValue));
}


void MainWindow::on_slider_progress_sliderMoved(int position)
{
    isRunning = false;
    stopLoop = true;
    ui->bt_run->setText("Run");
}

void MainWindow::plotChart(int currentColIndex){

    // Plot graphs using QCharts
    if (vizData->statsData[currentColIndex].median.isEmpty()) {
        return;
    }

    // Create a QChart object
    chart = new QChart();
    chart->setTitle(ui->cb_db_list->currentText());

    // Create a QLineSeries object
    series = new QLineSeries();

    // Populate the series with the median data
    for (int month = 0; month < vizData->statsData[currentColIndex].median.size(); month++) {
        series->append(month, vizData->statsData[currentColIndex].median[month][0]);  // Assuming location 0
    }

    // Add the series to the chart
    chart->addSeries(series);
    chart->legend()->hide();  // Hide the legend

    // Create axes
    axisX = new QValueAxis;
    axisX->setTitleText("Month");
    axisX->setTickCount(20);

    axisY = new QValueAxis;

    // Add the axes to the chart
    chart->setAxisX(axisX, series);
    chart->setAxisY(axisY, series);

    chart->setTheme(QChart::ChartThemeDark);

    // Create a QChartView to display the chart
    ui->gv_chartview->setRenderHint(QPainter::Antialiasing);
    ui->gv_chartview->setChart(chart);
}

double MainWindow::plotVerticalLineOnChart(int currentMonth) {
    if (!chart || !series) {
        return 0.0;  // Ensure chart and series are initialized before proceeding
    }

    // If a vertical line already exists, remove it from the chart
    if (currentVerticalLine) {
        chart->removeSeries(currentVerticalLine);
        delete currentVerticalLine;  // Clean up memory
        currentVerticalLine = nullptr;
    }

    // If a label exists, remove it from the scene
    if (valueLabel) {
        chart->scene()->removeItem(valueLabel);
        delete valueLabel;  // Clean up memory
        valueLabel = nullptr;
    }

    // Get the current range of the Y-axis
    QValueAxis *axisY = qobject_cast<QValueAxis *>(chart->axisY());
    if (!axisY) {
        return 0.0;
    }

    // Get the min and max values of the Y-axis
    qreal minY = axisY->min();
    qreal maxY = axisY->max();

    // Create a new QLineSeries for the vertical line
    currentVerticalLine = new QLineSeries();

    // Add two points to create a vertical line at the specified month
    currentVerticalLine->append(currentMonth, minY);  // Point at the bottom
    currentVerticalLine->append(currentMonth, maxY);  // Point at the top

    // Set the vertical line's appearance (e.g., dashed line, color)
    QPen linePen(Qt::SolidLine);  // Use a dashed line
    linePen.setColor(Qt::red);   // Set color to red, for example
    linePen.setWidth(2);         // Set line width
    currentVerticalLine->setPen(linePen);

    // Add the vertical line series to the chart
    chart->addSeries(currentVerticalLine);

    // Attach the axes to the vertical line
    chart->setAxisX(chart->axisX(), currentVerticalLine);
    chart->setAxisY(chart->axisY(), currentVerticalLine);

    // Now display the Y value at the point where the vertical line crosses the data series
    qreal yValue = vizData->statsData[currentColIndexPlaying].median[currentMonth][0];  // Get the Y-value at the specified month

    // Create a label to display the Y-value
    valueLabel = new QGraphicsSimpleTextItem(QString::number(yValue));

    // Find the position of the label on the chart (convert chart coordinates to scene coordinates)
    QPointF labelPos = chart->mapToPosition(QPointF(currentMonth, yValue), currentVerticalLine);

    // Set the position of the label slightly above the intersection point
    valueLabel->setPos(labelPos.x(), labelPos.y() - 20);  // Offset slightly upwards to avoid overlapping with the point

    valueLabel->setPen(QPen(Qt::white));
    valueLabel->setBrush(QBrush(Qt::white));

    // Add the label to the chart's scene
    chart->scene()->addItem(valueLabel);

    return yValue;
}

void MainWindow::on_cb_db_list_currentIndexChanged(int index)
{
    currentColIndexPlaying = index;
    plotChart(index);
    if(!isRunning){
        ui->openGLWidget->updateInstanceDataMedian(currentColIndexPlaying, currentMonth);
        QMetaObject::invokeMethod(ui->openGLWidget, "updateVertexBuffers", Qt::QueuedConnection);
        double yValue = plotVerticalLineOnChart(currentMonth);
        ui->statusbar->showMessage("Month: " + QString::number(currentMonth) + " Year: " + QString::number(currentMonth / 12) + " Value: " + QString::number(yValue));
    }

}

