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
#include <QFormLayout>
#include <QStandardItemModel>

#include "loadersqlite.h"
#include "loaderyml.h"
#include "loaderraster.h"
#include "chatbotwithapi.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Initialize preferences
    preference = new Preference("./config.ini");

    // Load window size and position on startup
    resize(preference->loadWindowSize());
    move(preference->loadWindowPosition());

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

    ui->le_sim_path->setText("");
    ui->le_sim_path->setPlaceholderText("Input simulation path then [Enter] or using [Browse] button");

    chart = new ChartCustom(this);
    chart->setChartView(ui->gv_chartview);
    chart->setVizData(vizData);

    currentColNameShown = "";
    currentMonth = 0;
    currentLocationSelectedMap = QMap<QPair<int,int>,QColor>();

    ui->wg_color_map->setHidden(true);

    ui->wev_chatbox->page()->setBackgroundColor(Qt::transparent);
    ui->wev_chatbox->setMinimumWidth(width()/3);
    ui->wev_chatbox->initChatScreen();
    ui->wev_chatbox->setEnabled(false);
    ui->wev_chatbox->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    ui->wev_chatbox->setHidden(true);
    ui->bt_chat_setting->setHidden(true);

    QObject::connect(ui->graphicsView, &GraphicsViewCustom::squareClickedOnScene, this, &MainWindow::onSquareClicked);
    QObject::connect(this, &MainWindow::addClearButton, ui->graphicsView, &GraphicsViewCustom::showClearButton);
    QObject::connect(this,&MainWindow::isAssisantReady, ui->wev_chatbox, &WebEngineViewCustom::isAssistantReady);

    hideMedianItems();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    ui->wev_chatbox->setMinimumWidth(width()/3);
    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Save window size and position
    preference->saveWindowSize(size());
    preference->saveWindowPosition(pos());

    // Optionally, save other preferences like paths here

    // Call the base class implementation
    QMainWindow::closeEvent(event);
}

QLayout* widgetToLayout(QWidget* w){
    auto layout = new QVBoxLayout();
    layout->addWidget( w );
    return layout;
}
QWidget* layoutToWidget(QLayout* l){
    auto widget = new QWidget();
    widget->setLayout( l );
    return widget;
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

bool MainWindow::displaySqlSelection(VizData* vizData, QWidget* parentWidget) {
    // Create the dialog
    QDialog* dialog = new QDialog(parentWidget);
    dialog->setWindowTitle("Select columns to plot");
    dialog->resize(600, 600); // Set an initial size for the dialog
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
    locationIdEdit->setMinimumWidth(400);  // Ensure the line edit expands

    QLineEdit* monthIdEdit = new QLineEdit(dialog);
    monthIdEdit->setPlaceholderText(vizData->sqlData.monthID + " (default)");
    monthIdEdit->setMinimumWidth(400);     // Ensure the line edit expands

    // Create labels for the LocationID and MonthID fields
    QLabel* locationIdLabel = new QLabel("Location column name:", dialog);
    QLabel* monthIdLabel = new QLabel("Month column name:", dialog);

    // Use QFormLayout for proper alignment and full width
    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(locationIdLabel, locationIdEdit); // Add label and corresponding edit field
    formLayout->addRow(monthIdLabel, monthIdEdit);       // Add label and corresponding edit field

    // Ensure the form layout fields grow to full width
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setFormAlignment(Qt::AlignLeft);

    // Create a combo box with two options: "District" and "Pixel"
    QComboBox* reporterType = new QComboBox(dialog);
    reporterType->addItem("Pixel");
    reporterType->addItem("District");

    QLabel* reporterTypeLabel = new QLabel("Select type:", dialog);

    // Create a layout for the combo box
    QHBoxLayout* reporterTypeLayout = new QHBoxLayout();
    reporterTypeLayout->addWidget(reporterTypeLabel);
    reporterTypeLayout->addWidget(reporterType);


    // Create a combo box with two options: "District" and "Pixel"
    QComboBox* rasterSelection = new QComboBox(dialog);
    QStringList ascFileNameList;
    for(const QString &ascFilePath : ascFileList){
        QFileInfo fileInfo(ascFilePath);
        ascFileNameList.append(fileInfo.fileName());
        cbItemPathMap[fileInfo.fileName()] = ascFilePath;
    }
    QStringListModel *model = new QStringListModel(this);
    model->setStringList(ascFileNameList);
    rasterSelection->setModel(model);

    QLabel* rasterSelectionLabel = new QLabel("Select district raster:", dialog);

    // Create a layout for the combo box
    QHBoxLayout* rasterSelectionLayout = new QHBoxLayout();
    rasterSelectionLayout->addWidget(rasterSelectionLabel);
    rasterSelectionLayout->addWidget(rasterSelection);
    rasterSelectionLabel->setVisible(false);
    rasterSelection->setVisible(false);
    QObject::connect(rasterSelection, &QComboBox::currentTextChanged, [=](const QString &text) {
        districtRasterPath = cbItemPathMap[text];
    });

    // Create a QDialogButtonBox with OK and Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect the OK button to dialog's accept slot and Cancel button to reject slot
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    QObject::connect(reporterType, &QComboBox::currentTextChanged, [=](const QString &text) {
        if(text == "District"){
            rasterSelectionLabel->setVisible(true);
            rasterSelection->setVisible(true);
            vizData->isDistrictReporter = true;
        }
        else{
            rasterSelectionLabel->setVisible(false);
            rasterSelection->setVisible(false);
            vizData->isDistrictReporter = false;
        }
    });

    // Create a layout for the dialog
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(tabWidget);         // Add the QTabWidget to the layout
    layout->addLayout(formLayout);        // Add the form layout containing LocationID and MonthID fields
    layout->addLayout(reporterTypeLayout);    // Add the combo box layout
    layout->addLayout(rasterSelectionLayout);    // Add the combo box layout
    layout->addWidget(buttonBox);         // Add the button box to the layout

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
        QString reportType = reporterType->currentText(); // Capture selected combo box option
        qDebug() << "Location :" << locationID;
        qDebug() << "Month ID:" << monthID;
        if(vizData->isDistrictReporter){
            qDebug() << "Reporter Type:" << reportType;
            qDebug() << "District map:" << districtRasterPath;
        }

        for (const QString tableName : checkBoxStatus.keys()) {
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

void MainWindow::showLastSquareValue(){
    if(currentLocationSelectedMap.isEmpty()){
        ui->statusbar->showMessage("No location selected");
    }
    else{
        int lastLocation = -1;
        if(vizData->isDistrictReporter){
            lastLocation = vizData->rasterData->locationPair2DTo1DDistrict[currentLocationSelectedMap.keys().last()];
        }
        else{
            lastLocation = vizData->rasterData->locationPair2DTo1D[currentLocationSelectedMap.keys().last()];
        }
        QPair<int,int> lastPos = vizData->rasterData->locationPair1DTo2D[lastLocation];
        double data = vizData->rasterData->raster->data[lastPos.first][lastPos.second];
        ui->statusbar->showMessage("Location: " + QString::number(lastLocation)
                                   + "(row: " + QString::number(lastPos.first) + ", col: " + QString::number(lastPos.second) + ")"
                                   + " Value: " + QString::number(data));
    }
}

void MainWindow::onSquareClicked(const QPoint &pos, const QColor &color)
{
    if(currentLocationSelectedMap.contains(QPair<int,int>(pos.y(),pos.x()))){
        currentLocationSelectedMap.remove(QPair<int,int>(pos.y(),pos.x()));
    }
    else{
        if(pos.x() == -1 && pos.y() == -1){
            currentLocationSelectedMap.clear();
            qDebug() << "[Main]Clear all locations";
        }
        else{
            currentLocationSelectedMap[QPair<int,int>(pos.y(),pos.x())] = color;
        }
    }

    qDebug() << "[Main]Square select at:" << pos << "loc: " << QPair<int,int>(pos.y(),pos.x()) << "color: " << color;
    if(screenNumber == 1){
        showChart();
    }
    else{
        showLastSquareValue();
    }

    emit(addClearButton(!currentLocationSelectedMap.empty()));
}

void MainWindow::onMouseMoved(const QPoint &pos)
{
}

void MainWindow::checkDirectory(QString selectedDirectory)
{
    all_rasters_exist = false;
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
        screenNumber = 0;
        resetMedianMap();
        enableInputWidgets(screenNumber);
        ui->le_sim_path->setText(selectedDirectory);
        vizData->currentDirectory = selectedDirectory;
        showItemScreenNumber(screenNumber);
    }
}

void MainWindow::on_le_sim_path_returnPressed()
{
    if(!isRunning){
        statusMessage = "No folder selected.";

        // Open a file dialog to select a folder
        QString selectedDirectory = ui->le_sim_path->text();

        checkDirectory(selectedDirectory);
    }
    else{
        QMessageBox::information(this, "Information", "Plese stop playing first!");
    }
}

void MainWindow::on_bt_auto_load_folder_clicked()
{
    if(!isRunning){
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
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks // Show only directories
            );

        checkDirectory(selectedDirectory);
    }
    else{
        QMessageBox::information(this, "Information", "Plese stop playing first!");
    }
}

void MainWindow::on_bt_process_clicked()
{

    if(!isRunning){
        loader = nullptr;
        loader = new LoaderYML();
        loader->loadFileSingle(ymlFileList[0], vizData, nullptr, nullptr);

        loader = nullptr;
        loader = new LoaderSQLite();
        loader->loadFileSingle(dbFileList[0], vizData, nullptr, nullptr);

        if(!displaySqlSelection(vizData, this)){
            return;
        }

        ui->statusbar->showMessage("Loading database files...");

        if(vizData->sqlData.tableColumnsMap.isEmpty()){
            QMessageBox::information(this, "Information", "No columns selected.");
            return;
        }

        if(vizData->isDistrictReporter){
            if(!districtRasterPath.isEmpty()){
                loader = nullptr;
                loader = new LoaderRaster();
                loader->loadFileSingle(districtRasterPath, vizData, nullptr, nullptr);
            }
        }

        disabeInputWidgets();
        loader = nullptr;
        loader = new LoaderSQLite();
        loader->loadDBList(dbFileList,
                           vizData->sqlData.locationID,
                           vizData->sqlData.monthID,
                           vizData->sqlData.tableColumnsMap[vizData->sqlData.tableColumnsMap.keys().last()],
                           vizData->sqlData.tableColumnsMap.keys().last(), vizData,
                           [this](int progress) {  // Progress callback
                               QMetaObject::invokeMethod(this, [this, progress]() {
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

                                   preference->saveDBPaths(dbFileList);

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
    else{
        QMessageBox::information(this, "Information", "Plese stop playing first!");
    }
}

void MainWindow::on_bt_run_clicked()
{
    isRunning = !isRunning;
    if (isRunning) {
        ui->bt_run->setText("Pause");
    } else {
        ui->bt_run->setText("Run");
    }

    qDebug() << "clicked run" << currentMonth;

    QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>(this);

    // Define a lambda function to update the data in a separate thread
    QFuture<void> future = QtConcurrent::run([=]() {
        for (int month = currentMonth; month < vizData->statsData[currentColNameShown].iqr[0].size(); month++) {
            // Check if the loop should be stopped
            if (!isRunning) {
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
        if (ui->slider_progress->value() == vizData->monthCountStartToEnd - 1) {
            // Reset the state when processing finishes
            // qDebug() << "Running finished!";
            resetPlayState();
        }
    });
}

void MainWindow::on_slider_progress_valueChanged(int value)
{
    currentMonth = value;
    if (currentMonth >= vizData->monthCountStartToEnd) {
        currentMonth = vizData->monthCountStartToEnd - 1;
    }
    updateMedianMap();
    showChart();
    ui->statusbar->showMessage("Month: " + QString::number(currentMonth + 1) + " Year: " + QString::number(currentMonth / 12));
    // qDebug() << value << "Month:" << currentMonth;
}

void MainWindow::on_slider_progress_sliderMoved(int value)
{
    ui->slider_progress->setValue(value);
}


void MainWindow::on_slider_progress_sliderPressed()
{
    isRunning = false;
    ui->bt_run->setText("Run");
    currentMonth = ui->slider_progress->value();
    qDebug() << "Slider pressed!" << "value:" << ui->slider_progress->value();
}


void MainWindow::on_slider_progress_sliderReleased()
{
    isRunning = false;
    ui->bt_run->setText("Run");
    currentMonth = ui->slider_progress->value();
    qDebug() << "Slider released!" << "value:" << ui->slider_progress->value();
}


void MainWindow::disabeInputWidgets(){
    ui->le_sim_path->setEnabled(false);
    ui->bt_process->setEnabled(false);
    ui->cb_data_list->setEnabled(false);
    ui->bt_auto_load_folder->setEnabled(false);
    ui->bt_run->setEnabled(false);
    ui->graphicsView->setEnabled(false);
    ui->slider_progress->setEnabled(false);
}

void MainWindow::enableInputWidgets(int screenNumber){
    if(screenNumber == 0){
        ui->le_sim_path->setEnabled(true);
        ui->bt_auto_load_folder->setEnabled(true);
        ui->bt_process->setEnabled(true);
        ui->cb_data_list->setEnabled(true);
        ui->graphicsView->setEnabled(true);
    }
    if(screenNumber == 1){
        ui->le_sim_path->setEnabled(true);
        ui->bt_auto_load_folder->setEnabled(true);
        ui->bt_process->setEnabled(true);
        ui->cb_data_list->setEnabled(true);
        ui->bt_run->setEnabled(true);
        ui->graphicsView->setEnabled(true);
        ui->slider_progress->setEnabled(true);
    }
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
    emit(addClearButton(!currentLocationSelectedMap.empty()));
}

void MainWindow::updateMedianMap(){
    ui->wg_color_map->setColorMapMinMax(QPair<double,double>(vizData->statsData[currentColNameShown].medianMin, vizData->statsData[currentColNameShown].medianMax));
    ui->graphicsView->updateRasterDataMedian(currentColNameShown, currentMonth);
    ui->graphicsView->update();
}

void MainWindow::showChart(){
    ui->gv_chartview->setHidden(currentLocationSelectedMap.isEmpty());
    if(ui->gv_chartview->isVisible()){
        chart->plotDataMedianMultipleLocations(currentColNameShown, currentLocationSelectedMap, currentMonth, ui->cb_data_list->currentText());
    }
}

void MainWindow::hideMedianItems(){
    ui->gv_chartview->setHidden(true);
}

void MainWindow::showItemScreenNumber(int screenNumber){
    if(screenNumber == 0){
        enableInputWidgets(screenNumber);
        hideMedianItems();
        resetPlayState();
        ui->slider_progress->setValue(0);
        ui->slider_progress->setEnabled(false);
        ui->bt_run->setEnabled(false);
        ui->wg_color_map->setHidden(false);
        ui->gv_chartview->setHidden(true);
        //Display only filenames in the combobox
        QStringList ascFileNameList;
        for(const QString &ascFilePath : ascFileList){
            QFileInfo fileInfo(ascFilePath);
            ascFileNameList.append(fileInfo.fileName());
            cbItemPathMap[fileInfo.fileName()] = ascFilePath;
        }
        QStringListModel *model = new QStringListModel(this);
        model->setStringList(ascFileNameList);
        ui->cb_data_list->setModel(model);
        ui->cb_data_list->setCurrentText(ascFileNameList.first());
        showMap(ascFileNameList.first());
        vizData->isDistrictReporter = false;
    }
    else if(screenNumber == 1){
        enableInputWidgets(screenNumber);
        hideMedianItems();
        resetPlayState();
        isRunning = false;
        ui->slider_progress->setValue(0);
        ui->slider_progress->setEnabled(true);
        ui->slider_progress->setMaximum(vizData->monthCountStartToEnd - 1);
        QStringListModel *model = new QStringListModel(this);
        QStringList columnList = vizData->statsData.keys();
        model->setStringList(columnList);
        ui->cb_data_list->setModel(model);
        ui->cb_data_list->setCurrentText(columnList.first());
        showMap(columnList.first());
    }
}

void MainWindow::resetPlayState(){
    isRunning = false;
    ui->bt_run->setText("Run");
    ui->slider_progress->setValue(0);
}

void MainWindow::saveStatsData(){
    dataProcessor->saveStatsDataToCSV(vizData,
                                      [this](int progress) {  // Progress callback
                                          QMetaObject::invokeMethod(this, [this, progress]() {
                                              ui->statusbar->showMessage("Saving IQR data to CSV ... " + QString::number(progress) + "%");
                                          }, Qt::QueuedConnection);
                                      },
                                      [this]() {  // Completion callback
                                          QMetaObject::invokeMethod(this, [this]() {
                                              qDebug() << "Saving IQR data to CSV complete!";
                                              ui->statusbar->showMessage("Saving IQR data to CSV complete!");

                                              resetMedianMap();
                                              updateMedianMap();
                                              screenNumber = 1;
                                              enableInputWidgets(screenNumber);
                                              showItemScreenNumber(screenNumber);
                                          }, Qt::QueuedConnection);
                                      });
}

void MainWindow::processAndSaveStatsData(){
    dataProcessor->processStatsData(vizData,[this](int progress) {  // Progress callback
                                                QMetaObject::invokeMethod(this, [this, progress]() {
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
                                                    updateMedianMap();
                                                    screenNumber = 1;
                                                    enableInputWidgets(screenNumber);
                                                    showItemScreenNumber(screenNumber);
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
                                                        qDebug() << screenNumber;
                                                        enableInputWidgets(screenNumber);
                                                    }
                                                    return;
                                                }
                                            }, Qt::QueuedConnection);
                                        });
}

void MainWindow::on_cb_data_list_currentTextChanged(const QString &name)
{
    showMap(name);
}

void MainWindow::showMap(QString name){
    if(screenNumber == 0){
        LoaderRaster *loader = new LoaderRaster();
        loader->loadFileSingle(cbItemPathMap[name], vizData, nullptr, nullptr);
        qDebug() << "ncols:" << vizData->rasterData->raster->NCOLS << " nrows:" << vizData->rasterData->raster->NROWS;
        ui->wg_color_map->setColorMapMinMax(QPair<double,double>(vizData->rasterData->dataMin, vizData->rasterData->dataMax));
        ui->graphicsView->updateRasterData();
        showLastSquareValue();
        preference->saveWorkPath(vizData->currentDirectory);
    }
    if(screenNumber == 1){
        qDebug() << "Column name changed to:" << name;
        currentColNameShown = name;
        updateMedianMap();
        showChart();
    }
}


// //select chatbot online or offline
// // Pass a path to an API key file or a direct API key
// QString apiKeyOrFilePath = "chatbot-api-key.txt";  // Can also be a string like "sk-xxxxxxxxxxxx"
// OnlineChatbot *chatbot = new OnlineChatbot(apiKeyOrFilePath);
// // Now you can use the chatbot to send messages
// QString response = chatbot->sendMessage("Hello, how are you?");
// qDebug() << "Response from chatbot:" << response;

bool MainWindow::displayChatbotSetting(VizData* vizData, QWidget* parentWidget){

    // Create dialog
    QDialog* dialog = new QDialog(parentWidget);
    dialog->setWindowTitle("Chatbot Settings");
    dialog->resize(400, 150); // Set an initial size for the dialog
    dialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    dialog->setFixedSize(QSize(400, 150)); // Set a fixed size for the dialog

    // Create form layout
    QFormLayout *formLayout = new QFormLayout(dialog);

    // Chatbot type combobox
    QLabel *labelType = new QLabel("Select Chatbot Type:", dialog);
    QComboBox *cbChatbotType = new QComboBox(dialog);
    cbChatbotType->addItem("Using API");
    cbChatbotType->addItem("Using local model");

    // Online input field for API key or path
    QLabel *labelApiKey = new QLabel("Enter API Key or Path:", dialog);
    QLineEdit *editApiKey = new QLineEdit(dialog);

    // Offline combobox for model selection
    QLabel *labelModel = new QLabel("Select Model:", dialog);
    QComboBox *cbModel = new QComboBox(dialog);
    cbModel->addItem("Model A");
    cbModel->addItem("Model B");
    cbModel->addItem("Model C");

    // Add OK and Cancel buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnOk = new QPushButton("OK", dialog);
    QPushButton *btnCancel = new QPushButton("Cancel", dialog);
    buttonLayout->addWidget(btnOk);
    buttonLayout->addWidget(btnCancel);

    editApiKey->setText(preference->loadAPIKeyPath(""));
    cbModel->setCurrentText(preference->loadModelPath(""));

    // Initially show API key input and hide model combobox
    labelApiKey->setVisible(true);
    editApiKey->setVisible(true);
    labelModel->setVisible(false);
    cbModel->setVisible(false);

    // Update visibility based on chatbot type selection
    connect(cbChatbotType, &QComboBox::currentIndexChanged, [&]() {
        if (cbChatbotType->currentText() == "Using API") {
            labelApiKey->setVisible(true);
            editApiKey->setVisible(true);
            labelModel->setVisible(false);
            cbModel->setVisible(false);
        } else if (cbChatbotType->currentText() == "Using local model") {
            labelApiKey->setVisible(false);
            editApiKey->setVisible(false);
            labelModel->setVisible(true);
            cbModel->setVisible(true);
        }
    });

    // Add widgets to form layout
    formLayout->addRow(labelType, cbChatbotType);
    formLayout->addRow(labelApiKey, editApiKey);
    formLayout->addRow(labelModel, cbModel);
    formLayout->addRow(buttonLayout);
    formLayout->setLabelAlignment(Qt::AlignmentFlag::AlignLeft);
    formLayout->setFormAlignment(Qt::AlignmentFlag::AlignRight);
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);

    // Handle OK button click
    connect(btnOk, &QPushButton::clicked, [&]() {
        if (cbChatbotType->currentText() == "Using API") {
            vizData->chatbotData.apiKey = editApiKey->text();  // Store API key/path
            vizData->chatbotData.isWithAPI = true;
        } else {
            vizData->chatbotData.modelPath = cbModel->currentText();  // Store selected offline model
            vizData->chatbotData.isWithAPI = false;
        }
        dialog->accept();  // Close dialog with OK
    });

    // Handle Cancel button click
    connect(btnCancel, &QPushButton::clicked, [&]() {
        dialog->reject();  // Close dialog with Cancel
    });

    // Show the dialog and wait for user interaction
    if (dialog->exec() == QDialog::Accepted) {
        // Settings confirmed, you can now use `chatBotType`, `chatBotOnlineAPIKeyOrPath`, and `chatBotOfflineModelPath`
        if (vizData->chatbotData.isWithAPI) {
            qDebug() << "Chatbot type: Using API, API Key/Path:" << vizData->chatbotData.apiKey;
            if(!vizData->chatbotData.apiKey.isEmpty()){
                preference->saveAPIKeyPath(vizData->chatbotData.apiKey);
                return true;
            }
        } else {
            qDebug() << "Chatbot type: Using model, Selected Model:" << vizData->chatbotData.modelPath;
            if(!vizData->chatbotData.modelPath.isEmpty()){
                preference->saveModelPath(vizData->chatbotData.modelPath);
                return true;
            }
        }
    } else {
        // User canceled the settings dialog
        return false;
    }
    return false;
}

void MainWindow::onChatbotReplyReceived(QString response){
    emit appendChatBotText(response);
}

void MainWindow::on_bt_chat_setting_clicked()
{
    if(displayChatbotSetting(vizData, this)){
        ui->wev_chatbox->setVizData(vizData);
        emit isAssisantReady(true);
        if(ui->wev_chatbox->isHidden()){
            ui->wev_chatbox->setHidden(false);
        }
    }
}


void MainWindow::on_chb_assist_clicked(bool checked)
{
    if(checked){
        ui->bt_chat_setting->setHidden(false);
        if(ui->wev_chatbox->isHidden()){
            ui->wev_chatbox->setHidden(false);
        }

        // QWebEngineView* devToolsView = new QWebEngineView();
        // ui->wev_chatbox->page()->setDevToolsPage(devToolsView->page());
        // devToolsView->show();
    }
    else{
        ui->bt_chat_setting->setHidden(true);
        if(!ui->wev_chatbox->isHidden()){
            ui->wev_chatbox->setHidden(true);
        }
    }
}

