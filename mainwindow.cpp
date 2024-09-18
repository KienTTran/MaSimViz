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
    stopLoop = false;

    scene = new QGraphicsScene(this);
    chart = new ChartCustom(this);
    chart->setChartView(ui->gv_chartview);

    currentColIndexPlaying = 0;
    currentMonth = 0;
    currentLocationSelectedMap = QMap<int,QColor>();

    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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

void MainWindow::on_cb_raster_list_currentIndexChanged(int index)
{
    LoaderRaster *loader = new LoaderRaster();
    loader->loadFileSingle(ascFileList[index], vizData, nullptr, nullptr);

    qDebug() << "ncols:" << vizData->rasterData->raster->NCOLS << " nrows:" << vizData->rasterData->raster->NROWS;

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::TextAntialiasing);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsView->displayAscData(scene, vizData);

    QObject::connect(ui->graphicsView, &GraphicsViewCustom::squareClickedOnScene, this, &MainWindow::onSquareClicked);
}

void MainWindow::onSquareClicked(const QPoint &pos, const QColor &color)
{
    int location = vizData->rasterData->locationPair2DTo1D[QPair<int,int>(pos.y(),pos.x())];
    if(currentLocationSelectedMap.contains(location)){
        currentLocationSelectedMap.remove(location);
    }
    else{
        currentLocationSelectedMap[location] = color;
    }

    qDebug() << "[Main]Square select at:" << pos << "loc: " << location << "color: " << color;
    if(ui->cb_db_list->isVisible()){
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
        showItemsAfterBrowseClicked();

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

    resetMedianMap();
    disabeInputWidgets();    
    showItemsAfterProcessClicked();

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
                                                                       ui->cb_db_list->setModel(model);

                                                                       resetMedianMap();
                                                                       showItemsAfterProcessClicked();
                                                                       showMedianMap();
                                                                       enableInputWidgets();
                                                                   }, Qt::QueuedConnection);
                                                               });
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
        for (int month = currentMonth; month < vizData->statsData[currentColIndexPlaying].median.size(); month++) {
            // Check if the loop should be stopped
            if (stopLoop) {
                currentMonth = month;
                break;
            }

            // Progress bar needs to be updated in the main thread
            QMetaObject::invokeMethod(this, [=]() {
                ui->slider_progress->setValue(month * 100 / vizData->statsData[currentColIndexPlaying].median.size());
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
            resetPlayState();
        }
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
    currentMonth = value * vizData->statsData[currentColIndexPlaying].median.size() / 100;
    if (currentMonth >= vizData->statsData[currentColIndexPlaying].median.size()) {
        currentMonth = vizData->statsData[currentColIndexPlaying].median.size() - 1;
    }
    showMedianMap();
    showChart();
    ui->statusbar->showMessage("Month: " + QString::number(currentMonth) + " Year: " + QString::number(currentMonth / 12));
}


void MainWindow::on_slider_progress_sliderMoved(int position)
{
    isRunning = false;
    stopLoop = true;
}

void MainWindow::on_cb_db_list_currentIndexChanged(int index)
{
    currentColIndexPlaying = index;
    showMedianMap();
    showChart();
}

void MainWindow::disabeInputWidgets(){
    ui->le_sim_path->setEnabled(false);
    ui->bt_process->setEnabled(false);
    ui->cb_raster_list->setEnabled(false);
    ui->bt_auto_load_folder->setEnabled(false);
    ui->bt_run->setEnabled(false);
    ui->graphicsView->setEnabled(false);
}

void MainWindow::enableInputWidgets(){
    ui->le_sim_path->setEnabled(true);
    ui->bt_process->setEnabled(true);
    ui->cb_raster_list->setEnabled(true);
    ui->bt_auto_load_folder->setEnabled(true);
    ui->progress_bar->setValue(0);
    ui->bt_run->setEnabled(true);
    ui->graphicsView->setEnabled(true);
}

void MainWindow::resetMedianMap(){
    currentColIndexPlaying = 0;
    currentMonth = 0;
    currentLocationSelectedMap.clear();
    ui->graphicsView->resetGraphicsView();
    ui->gv_chartview->setHidden(currentLocationSelectedMap.isEmpty());
}

void MainWindow::showMedianMap(){
    ui->graphicsView->displayAscDataMedian(scene, currentColIndexPlaying, vizData, currentMonth);
    ui->graphicsView->update();
}

void MainWindow::showChart(){
    ui->gv_chartview->setHidden(currentLocationSelectedMap.isEmpty());
    if(ui->gv_chartview->isVisible()){
        chart->plotDataMedianMultipleLocations(vizData, currentColIndexPlaying, currentLocationSelectedMap, currentMonth, ui->cb_db_list->currentText());
    }
}

void MainWindow::hideMedianItems(){
    ui->slider_progress->setHidden(true);
    ui->cb_db_list->setHidden(true);
    ui->gv_chartview->setHidden(true);
}

void MainWindow::showItemsAfterBrowseClicked(){
    ui->cb_raster_list->setHidden(false);
    ui->cb_db_list->setHidden(ui->cb_raster_list->isVisible());
    ui->slider_progress->setHidden(true);
    ui->progress_bar->setHidden(ui->slider_progress->isVisible());
    ui->slider_progress->setValue(0);
    ui->progress_bar->setValue(0);
    ui->bt_run->setEnabled(false);
}

void MainWindow::showItemsAfterProcessClicked(){
    ui->cb_db_list->setHidden(false);
    ui->cb_raster_list->setHidden(ui->cb_db_list->isVisible());
    ui->slider_progress->setHidden(true);
    ui->progress_bar->setHidden(ui->slider_progress->isVisible());
    ui->slider_progress->setValue(0);
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
    ui->statusbar->showMessage("Playing complete.");
}


