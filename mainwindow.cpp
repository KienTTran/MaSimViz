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
#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

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

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QDebug>
#include <QList>

// Function to display sqlData in a dialog with checkable columns in a checklist per tab
void displaySqlDataInDialogWithChecklist(VizData* vizData, QWidget* parentWidget = nullptr) {
    // Create the dialog
    QDialog* dialog = new QDialog(parentWidget);
    dialog->setWindowTitle("SQL Data Viewer with Checklists");
    dialog->resize(600, 400); // Set an initial size for the dialog

    // Create a QTabWidget to display the tables
    QTabWidget* tabWidget = new QTabWidget(dialog);

    // Create a table widget to hold the checkboxes and column names
    QTableWidget* tableWidget = new QTableWidget();
    tableWidget->setColumnCount(2); // Two columns: one for the checkbox, one for the column name
    tableWidget->setHorizontalHeaderLabels(QStringList() << "Select" << "Column Names");
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Stretch columns to fit

    QMap<QString,QList<bool>> checkBoxStatus;

    // Preload the state for each table
    for (int dbIndex = 0; dbIndex < vizData->sqlData.dbPaths.size(); ++dbIndex) {
        QStringList tables = vizData->sqlData.dbTables;
        checkBoxStatus = QMap<QString,QList<bool>>();
        for (const QString& tableName : tables) {
            int tableIndex = vizData->sqlData.dbTables.indexOf(tableName);
            int columnCount = vizData->sqlData.dbColumns[tableIndex].size();
            if (columnCount == 0) {
                qDebug() << "No columns found for table:" << tableName;
                continue;
            }

            // Initialize the checkbox status (all unchecked initially)
            checkBoxStatus[tableName] = QList<bool>(columnCount, false);
            tabWidget->addTab(new QWidget(), tableName);  // Add a placeholder tab
        }
    }

    // Slot to update the checklist when the tab is changed
    QObject::connect(tabWidget, &QTabWidget::currentChanged, [&](int index) {
        QString tableName = tabWidget->tabText(index);
        int tableIndex = vizData->sqlData.dbTables.indexOf(tableName);

        int columnCount = vizData->sqlData.dbColumns[tableIndex].size();
        if (columnCount == 0) {
            qDebug() << "No columns found for table:" << tableName;
            return;
        }

        // Clear the previous contents
        tableWidget->clearContents();
        tableWidget->setRowCount(columnCount); // Update the number of rows for the current table

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
    });



    // Trigger the update for the initial tab
    tabWidget->setCurrentIndex(0);  // Make sure the first tab is populated initially
    emit tabWidget->currentChanged(0);  // Ensure the first tab content is updated on load

    // Create a QDialogButtonBox with OK and Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect the OK button to dialog's accept slot and Cancel button to reject slot
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // Create a layout for the dialog
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(tabWidget);   // Add the QTabWidget to the layout
    layout->addWidget(tableWidget); // Add the checklist table widget below the tab
    layout->addWidget(buttonBox);   // Add the button box to the layout

    // Set the layout for the dialog
    dialog->setLayout(layout);

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
                if(checkBoxStatus[tableName][j]){
                    selectedColumns += vizData->sqlData.dbColumns[tableIndex][j] + ",";
                }
            }
            if(!selectedColumns.isEmpty()){
                selectedColumns.chop(1); // Remove the last comma
                vizData->sqlData.tableColumnMap[tableName] = selectedColumns;
            }
        }
        for (const QString& tableName : vizData->sqlData.tableColumnMap.keys()) {
            qDebug() << "Table:" << tableName << " Columns:" << vizData->sqlData.tableColumnMap[tableName];

        }
    } else {
        qDebug() << "Dialog rejected";  // Add this for debugging if Cancel is pressed or dialog is closed
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

    loader->loadFileSingle(dbFileList[0], vizData, nullptr, nullptr);

    displaySqlDataInDialogWithChecklist(vizData, this);

    if(vizData->sqlData.tableColumnMap.isEmpty()){
        QMessageBox::information(this, "Information", "No columns selected.");
        enableInputWidgets();
        return;
    }

    loader->loadDBList(dbFileList,
                       "locationid",
                       "monthlydataid",
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
                                                                   }, Qt::QueuedConnection);
                                                               });

                               enableInputWidgets();
                           }, Qt::QueuedConnection);
                       });
}


void MainWindow::on_bt_run_clicked()
{
    qDebug() << "Min: " << vizData->statsData[0].medianMin << " Max: " << vizData->statsData[0].medianMax;

    QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>(this);

    // Define a lambda function to update the data in a separate thread
    QFuture<void> future = QtConcurrent::run([=]() {
        for (int month = 0; month < vizData->statsData[0].median.size(); month++) {
            // Update raster data with the new PfPR values
            ui->openGLWidget->updateInstanceDataMedian(vizData, 0, month);

            // Ensure updateVertexBuffers() is called in the main thread
            QMetaObject::invokeMethod(ui->openGLWidget, "updateVertexBuffers", Qt::QueuedConnection);
            // qDebug() << "Month: " << month;

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

