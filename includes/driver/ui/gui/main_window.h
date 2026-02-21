#pragma once

#include <QMainWindow>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QColor>

/// @brief Main window for the picoc GUI compiler.
///
/// Features a modern dark theme, branded menu-bar header, split-pane layout
/// with compiler settings on the left and a live log panel on the right,
/// and a status bar with version info at the bottom.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void OnAddFiles();
    void OnRemoveFiles();
    void OnCompile();
    void OnClearLog();
    void OnBrowseOutputDir();
    void OnShowUsage();
    void OnShowTerminalSettings();

private:
    // ---- setup ----
    void SetupUI();
    void ApplyTheme();

    /// @brief Append a message to the log output area.
    void AppendLog(const QString& message);

    /// @brief Apply the current terminal colors to the log output (and usage dialog).
    void ApplyTerminalColors();

    // ---- returns the full QSS stylesheet string ----
    static QString GetStyleSheet();

    // ---- UI builder helpers ----
    QWidget*     CreateMenuBar();
    QGroupBox*   CreateInputFilesGroup();
    QGroupBox*   CreateOutputGroup();
    QGroupBox*   CreateOptimizationGroup();
    QGroupBox*   CreateWarningGroup();
    QGroupBox*   CreateLoggingGroup();
    QGroupBox*   CreateCompilationTypeGroup();
    QWidget*     CreateLogPanel();
    QPushButton* CreateCompileButton();
    QWidget*     CreateStatusBar();

    // ---- Input/Output widgets ----
    QListWidget* m_fileList       = nullptr;
    QLineEdit*   m_outputNameEdit = nullptr;
    QLineEdit*   m_outputDirEdit  = nullptr;

    // ---- Flag widgets ----
    QComboBox* m_optimizationCombo = nullptr;
    QComboBox* m_warningCombo      = nullptr;
    QCheckBox* m_verboseCheck      = nullptr;
    QCheckBox* m_quietCheck        = nullptr;
    QCheckBox* m_saveLogsCheck     = nullptr;
    QCheckBox* m_stopAsmCheck      = nullptr;
    QCheckBox* m_stopBinCheck      = nullptr;
    QCheckBox* m_saveTempsCheck    = nullptr;

    // ---- Action widgets ----
    QPushButton* m_compileButton = nullptr;
    QTextEdit*   m_logOutput     = nullptr;

    // ---- Terminal colour settings ----
    QColor m_termTextColor = QColor("#4ade80");   // default green
    QColor m_termBgColor   = QColor("#0d1117");   // default dark
};
