#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStatusBar>
#include <QDateTime>
#include <memory>

#include "HGTCollection.h"
#include "GeoidQuery.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectHGTFolder();
    void onSelectGeoidFile();
    void onQuery();
    void onClearResults();
    void onExit();
    void updateQueryButtonState();

private:
    void setupUI();
    void setupConnections();
    void updateStatus(const QString& message, bool isError = false);
    void appendToResult(const QString& text);
    void clearResult();
    bool validateInputs(double& lon, double& lat);

    // UI 组件
    QLineEdit* m_hgtFolderEdit;
    QLineEdit* m_geoidFileEdit;
    QLineEdit* m_lonEdit;
    QLineEdit* m_latEdit;
    QComboBox* m_methodCombo;
    QPushButton* m_queryBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_exitBtn;
    QPushButton* m_hgtBrowseBtn;
    QPushButton* m_geoidBrowseBtn;
    QTextEdit* m_resultDisplay;
    QLabel* m_statusLabel;
    QLabel* m_hgtCountLabel;

    // 数据对象
    std::unique_ptr<HGTCollection> m_dem;
    std::unique_ptr<GeoidQuery> m_geoid;

    QString m_hgtFolderPath;
    QString m_geoidFilePath;
    bool m_hgtLoaded;
};

#endif // MAINWINDOW_H
