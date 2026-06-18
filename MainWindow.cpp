#include "MainWindow.h"
#include <QApplication>
#include <QStyle>
#include <QDoubleValidator>
#include <QFont>
#include <QScrollBar>
#include <QDebug>
#include <cmath>
#include <sstream>
#include <iomanip>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_hgtLoaded(false)
{
    setupUI();
    setupConnections();
    updateStatus("Please select HGT folder to use", false);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    setWindowTitle("HGT 高程查询工具");
    setMinimumSize(950, 750);
    setWindowIcon(QIcon());

    // 中央窗口
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ==========================================
    // 输入区域
    // ==========================================
    QGroupBox* inputGroup = new QGroupBox("Input Parameters", this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(12);

    // 1. HGT 文件夹选择
    QHBoxLayout* hgtLayout = new QHBoxLayout();
    QLabel* hgtLabel = new QLabel("HGT Folder:", this);
    hgtLabel->setMinimumWidth(130);
    hgtLabel->setStyleSheet("font-weight: bold;");

    m_hgtFolderEdit = new QLineEdit(this);
    m_hgtFolderEdit->setPlaceholderText("Select folder containing .hgt files");
    m_hgtFolderEdit->setReadOnly(true);
    m_hgtFolderEdit->setStyleSheet("QLineEdit { background-color: #f8f8f8; }");

    m_hgtBrowseBtn = new QPushButton("Browse...", this);
    m_hgtBrowseBtn->setObjectName("browseBtn");
    m_hgtBrowseBtn->setFixedWidth(80);

    hgtLayout->addWidget(hgtLabel);
    hgtLayout->addWidget(m_hgtFolderEdit);
    hgtLayout->addWidget(m_hgtBrowseBtn);
    inputLayout->addLayout(hgtLayout);

    // HGT 文件计数
    m_hgtCountLabel = new QLabel("No HGT files loaded", this);
    m_hgtCountLabel->setStyleSheet("color: #888; font-style: italic; padding-left: 10px;");
    inputLayout->addWidget(m_hgtCountLabel);

    // 2. Geoid 文件选择
    QHBoxLayout* geoidLayout = new QHBoxLayout();
    QLabel* geoidLabel = new QLabel("EGM96 Geoid Grid:", this);
    geoidLabel->setMinimumWidth(130);
    geoidLabel->setStyleSheet("font-weight: bold;");

    m_geoidFileEdit = new QLineEdit(this);
    m_geoidFileEdit->setPlaceholderText("Optional: Select geoid grid file (e.g., .tif)");
    m_geoidFileEdit->setReadOnly(true);
    m_geoidFileEdit->setStyleSheet("QLineEdit { background-color: #f8f8f8; }");

    m_geoidBrowseBtn = new QPushButton("Browse...", this);
    m_geoidBrowseBtn->setObjectName("browseBtn");
    m_geoidBrowseBtn->setFixedWidth(80);

    geoidLayout->addWidget(geoidLabel);
    geoidLayout->addWidget(m_geoidFileEdit);
    geoidLayout->addWidget(m_geoidBrowseBtn);
    inputLayout->addLayout(geoidLayout);

    // 3. 经纬度输入
    QGridLayout* coordLayout = new QGridLayout();
    coordLayout->setSpacing(10);

    QLabel* lonLabel = new QLabel("Longitude lon:", this);
    lonLabel->setStyleSheet("font-weight: bold;");
    m_lonEdit = new QLineEdit(this);
    m_lonEdit->setPlaceholderText("e.g., 116.397");
    m_lonEdit->setValidator(new QDoubleValidator(-180, 180, 6, this));
    m_lonEdit->setMinimumWidth(150);

    QLabel* latLabel = new QLabel("Latitude lat:", this);
    latLabel->setStyleSheet("font-weight: bold;");
    m_latEdit = new QLineEdit(this);
    m_latEdit->setPlaceholderText("e.g., 39.908");
    m_latEdit->setValidator(new QDoubleValidator(-90, 90, 6, this));
    m_latEdit->setMinimumWidth(150);

    coordLayout->addWidget(lonLabel, 0, 0);
    coordLayout->addWidget(m_lonEdit, 0, 1);
    coordLayout->addWidget(latLabel, 0, 2);
    coordLayout->addWidget(m_latEdit, 0, 3);
    coordLayout->setColumnStretch(1, 1);
    coordLayout->setColumnStretch(3, 1);
    inputLayout->addLayout(coordLayout);

    // 4. 取值方式
    QHBoxLayout* methodLayout = new QHBoxLayout();
    QLabel* methodLabel = new QLabel("Interpolation:", this);
    methodLabel->setMinimumWidth(130);
    methodLabel->setStyleSheet("font-weight: bold;");

    m_methodCombo = new QComboBox(this);
    m_methodCombo->addItem("bilinear (Bilinear Interpolation)");
    m_methodCombo->addItem("nearest (Nearest Neighbor)");
    m_methodCombo->setCurrentIndex(0);
    m_methodCombo->setMinimumWidth(200);

    methodLayout->addWidget(methodLabel);
    methodLayout->addWidget(m_methodCombo);
    methodLayout->addStretch();
    inputLayout->addLayout(methodLayout);

    mainLayout->addWidget(inputGroup);

    // ==========================================
    // 按钮区域
    // ==========================================
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    m_queryBtn = new QPushButton("Query", this);
    m_queryBtn->setObjectName("queryBtn");
    m_queryBtn->setEnabled(false);
    m_queryBtn->setMinimumHeight(45);
    m_queryBtn->setStyleSheet("QPushButton { font-weight: bold; font-size: 14px; }");

    m_clearBtn = new QPushButton("Clear Results", this);
    m_clearBtn->setObjectName("clearBtn");
    m_clearBtn->setMinimumHeight(45);

    m_exitBtn = new QPushButton("Exit", this);
    m_exitBtn->setObjectName("exitBtn");
    m_exitBtn->setMinimumHeight(45);

    btnLayout->addStretch();
    btnLayout->addWidget(m_queryBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addWidget(m_exitBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    // ==========================================
    // 结果区域
    // ==========================================
    QGroupBox* resultGroup = new QGroupBox("Query Results", this);
    QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);

    m_resultDisplay = new QTextEdit(this);
    m_resultDisplay->setReadOnly(true);
    m_resultDisplay->setFont(QFont("Consolas", 10));
    m_resultDisplay->setMinimumHeight(250);
    m_resultDisplay->setStyleSheet(
        "QTextEdit { "
        "   background-color: #f8f8f8; "
        "   border: 1px solid #ddd; "
        "   border-radius: 4px; "
        "   font-family: Consolas, monospace; "
        "}"
    );

    resultLayout->addWidget(m_resultDisplay);
    mainLayout->addWidget(resultGroup);

    // ==========================================
    // 说明区域
    // ==========================================
    QTextEdit* helpText = new QTextEdit(this);
    helpText->setReadOnly(true);
    helpText->setMaximumHeight(100);
    helpText->setStyleSheet(
        "QTextEdit { "
        "   background-color: #fafafa; "
        "   border: 1px solid #ddd; "
        "   border-radius: 4px; "
        "   color: #555; "
        "   font-size: 11px; "
        "}"
    );
    helpText->setPlainText(
        "Instructions:\n"
        "1. First select the HGT folder containing valid .hgt files\n"
        "2. Geoid file is optional; if not selected, only HGT elevation H is output\n"
        "3. If geoid file is selected, N and WGS84 ellipsoidal height h = H + N are also output\n"
        "4. Bilinear uses bilinear interpolation, nearest uses nearest neighbor interpolation"
    );
    mainLayout->addWidget(helpText);

    // ==========================================
    // 状态栏
    // ==========================================
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("QLabel { padding: 5px 10px; }");
    statusBar()->addWidget(m_statusLabel);

    // ==========================================
    // 应用全局样式
    // ==========================================
    setStyleSheet(R"(
        QMainWindow {
            background-color: #ffffff;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #d0d0d0;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 12px;
            background-color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px 0 8px;
            color: #333;
        }
        QPushButton#queryBtn {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
        }
        QPushButton#queryBtn:hover {
            background-color: #45a049;
        }
        QPushButton#queryBtn:pressed {
            background-color: #3d8b40;
        }
        QPushButton#queryBtn:disabled {
            background-color: #c0c0c0;
            color: #888;
        }
        QPushButton#clearBtn {
            background-color: #FF9800;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
        }
        QPushButton#clearBtn:hover {
            background-color: #e68900;
        }
        QPushButton#clearBtn:pressed {
            background-color: #cc7a00;
        }
        QPushButton#exitBtn {
            background-color: #f44336;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
        }
        QPushButton#exitBtn:hover {
            background-color: #da190b;
        }
        QPushButton#exitBtn:pressed {
            background-color: #c2170a;
        }
        QPushButton#browseBtn {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 3px;
            padding: 6px 15px;
            font-weight: bold;
        }
        QPushButton#browseBtn:hover {
            background-color: #0b7dda;
        }
        QPushButton#browseBtn:pressed {
            background-color: #0a6ebd;
        }
        QLineEdit {
            padding: 6px 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
        QLineEdit:focus {
            border: 2px solid #4CAF50;
            background-color: #fafffe;
        }
        QLineEdit:disabled {
            background-color: #f0f0f0;
            color: #888;
        }
        QComboBox {
            padding: 6px 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
        QComboBox:focus {
            border: 2px solid #4CAF50;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #666;
            margin-right: 5px;
        }
        QLabel {
            color: #333;
        }
    )");
}

void MainWindow::setupConnections()
{
    // 浏览按钮连接
    connect(m_hgtBrowseBtn, &QPushButton::clicked, this, &MainWindow::onSelectHGTFolder);
    connect(m_geoidBrowseBtn, &QPushButton::clicked, this, &MainWindow::onSelectGeoidFile);

    // 功能按钮连接
    connect(m_queryBtn, &QPushButton::clicked, this, &MainWindow::onQuery);
    connect(m_clearBtn, &QPushButton::clicked, this, &MainWindow::onClearResults);
    connect(m_exitBtn, &QPushButton::clicked, this, &MainWindow::onExit);

    // 当经纬度输入变化时，更新查询按钮状态
    connect(m_lonEdit, &QLineEdit::textChanged, this, &MainWindow::updateQueryButtonState);
    connect(m_latEdit, &QLineEdit::textChanged, this, &MainWindow::updateQueryButtonState);
}

void MainWindow::onSelectHGTFolder()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select HGT Folder",
        m_hgtFolderPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (dir.isEmpty()) return;

    m_hgtFolderPath = dir;
    m_hgtFolderEdit->setText(dir);

    try {
        m_dem = std::make_unique<HGTCollection>(dir.toStdString());
        size_t count = m_dem->tileCount();

        if (count == 0) {
            updateStatus("No .hgt files found", true);
            m_hgtCountLabel->setText("[X] No HGT files found");
            m_hgtLoaded = false;
            m_queryBtn->setEnabled(false);
            return;
        }

        m_hgtLoaded = true;
        m_hgtCountLabel->setText(QString("[OK] Loaded %1 HGT tiles").arg(count));
        updateStatus(QString("Successfully loaded %1 HGT tiles").arg(count), false);
        updateQueryButtonState();

        clearResult();
        appendToResult("========================================");
        appendToResult("  System Initialization Successful");
        appendToResult(QString("  Loaded HGT tiles: %1").arg(count));
        appendToResult("  Folder: " + dir);
        appendToResult("========================================");

    }
    catch (const std::exception& e) {
        updateStatus(QString("Failed to load HGT files: %1").arg(e.what()), true);
        m_hgtLoaded = false;
        m_queryBtn->setEnabled(false);
        m_hgtCountLabel->setText("[X] Load failed");
    }
}

void MainWindow::onSelectGeoidFile()
{
    QString file = QFileDialog::getOpenFileName(
        this,
        "Select Geoid Grid File",
        m_geoidFilePath,
        "Raster files (*.tif *.tiff *.img *.hdr);;All files (*.*)"
    );

    if (file.isEmpty()) return;

    m_geoidFilePath = file;
    m_geoidFileEdit->setText(file);

    try {
        m_geoid = std::make_unique<GeoidQuery>(file.toStdString());
        if (m_geoid->isLoaded()) {
            updateStatus("Geoid file loaded successfully", false);
            appendToResult("[OK] Geoid file loaded successfully: " + file);
        }
        else {
            m_geoid.reset();
            updateStatus("Failed to load Geoid file", true);
            appendToResult("[X] Geoid file load failed: " + file);
        }
    }
    catch (const std::exception& e) {
        m_geoid.reset();
        updateStatus(QString("Failed to load Geoid file: %1").arg(e.what()), true);
        appendToResult("[X] Geoid file load exception: " + QString(e.what()));
    }
}

void MainWindow::onQuery()
{
    if (!m_hgtLoaded || !m_dem) {
        updateStatus("Please load HGT folder first", true);
        return;
    }

    double lon, lat;
    if (!validateInputs(lon, lat)) {
        return;
    }

    // 获取插值方法
    QString methodStr = m_methodCombo->currentText().contains("nearest") ? "nearest" : "bilinear";

    try {
        updateStatus("Querying...", false);
        QApplication::setOverrideCursor(Qt::WaitCursor);

        double H = m_dem->samplePoint(lon, lat, methodStr.toStdString());

        clearResult();
        appendToResult("========================================");
        appendToResult("          QUERY RESULTS");
        appendToResult("========================================");
        appendToResult(QString("  Longitude lon: %1").arg(lon, 0, 'f', 6));
        appendToResult(QString("  Latitude lat: %1").arg(lat, 0, 'f', 6));
        appendToResult(QString("  Interpolation: %1").arg(methodStr));
        appendToResult("");

        // ============================================
        // 输出 HGT 高程 H（始终输出）
        // ============================================
        if (std::isnan(H)) {
            appendToResult("  [X] HGT Elevation H (EGM96 Orthometric) = NaN (Invalid data)");
        }
        else {
            appendToResult(QString("  HGT Elevation H (EGM96 Orthometric) = %1 m").arg(H, 0, 'f', 3));
        }

        // ============================================
        // 如果加载了 Geoid 文件，输出 N 和 h
        // 如果没有加载，只显示提示信息
        // ============================================
        if (m_geoid && m_geoid->isLoaded()) {
            appendToResult("");
            appendToResult("  --- Geoid Correction ---");

            double N = m_geoid->query(lon, lat);
            if (std::isnan(N)) {
                appendToResult("  [X] Geoid Undulation N = NaN");
                appendToResult("  [X] WGS84 Ellipsoidal Height h = NaN");
            }
            else {
                double h = H + N;
                appendToResult(QString("  Geoid Undulation N = %1 m").arg(N, 0, 'f', 3));
                appendToResult(QString("  WGS84 Ellipsoidal Height h = H + N = %1 m").arg(h, 0, 'f', 3));
            }
        }
        else {
            // Geoid 文件未加载，只输出 H
            appendToResult("");
            appendToResult("  [i] Geoid file: Not loaded");
            appendToResult("  [i] Only HGT orthometric height H is output");
            appendToResult("  [i] To get WGS84 ellipsoidal height, load a geoid grid file");
        }

        appendToResult("");
        appendToResult("========================================");
        appendToResult(QString("  Query time: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        appendToResult("========================================");

        updateStatus("Query completed", false);

    }
    catch (const std::exception& e) {
        updateStatus(QString("Query failed: %1").arg(e.what()), true);
        appendToResult("[X] Query exception: " + QString(e.what()));
    }

    QApplication::restoreOverrideCursor();
}

void MainWindow::onClearResults()
{
    clearResult();
    m_lonEdit->clear();
    m_latEdit->clear();
    updateStatus("Results cleared", false);
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::updateStatus(const QString& message, bool isError)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString statusMsg = QString("[%1] %2").arg(timestamp, message);

    if (isError) {
        m_statusLabel->setText(statusMsg);
        m_statusLabel->setStyleSheet("QLabel { padding: 5px 10px; color: #d32f2f; font-weight: bold; }");
    }
    else {
        m_statusLabel->setText(statusMsg);
        m_statusLabel->setStyleSheet("QLabel { padding: 5px 10px; color: #2e7d32; }");
    }
}

void MainWindow::appendToResult(const QString& text)
{
    m_resultDisplay->append(text);
    // 自动滚动到底部
    QScrollBar* scrollbar = m_resultDisplay->verticalScrollBar();
    if (scrollbar) {
        scrollbar->setValue(scrollbar->maximum());
    }
}

void MainWindow::clearResult()
{
    m_resultDisplay->clear();
}

bool MainWindow::validateInputs(double& lon, double& lat)
{
    bool lonOk, latOk;
    lon = m_lonEdit->text().toDouble(&lonOk);
    lat = m_latEdit->text().toDouble(&latOk);

    if (!lonOk || !latOk) {
        updateStatus("Please enter valid longitude and latitude values", true);
        return false;
    }

    if (lon < -180 || lon > 180) {
        updateStatus("Longitude out of range (-180 ~ 180)", true);
        return false;
    }

    if (lat < -90 || lat > 90) {
        updateStatus("Latitude out of range (-90 ~ 90)", true);
        return false;
    }

    return true;
}

void MainWindow::updateQueryButtonState()
{
    bool hasLon = !m_lonEdit->text().isEmpty();
    bool hasLat = !m_latEdit->text().isEmpty();
    m_queryBtn->setEnabled(m_hgtLoaded && hasLon && hasLat);
}