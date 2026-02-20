#pragma once

#include <QMainWindow>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    /// @brief Open file dialog to add .picoc input files.
    void OnAddFiles();

    /// @brief Remove selected files from the input list.
    void OnRemoveFiles();

    /// @brief Build CompilerOptions from UI, validate, and compile.
    void OnCompile();

    /// @brief Clear the log output area.
    void OnClearLog();

    /// @brief Open folder dialog to choose output directory.
    void OnBrowseOutputDir();

private:
    /// @brief Build the entire UI layout by composing the helper methods below.
    void SetupUI();

    /// @brief Append a message to the log output area.
    void AppendLog(const QString& message);

    // ---- UI builder helpers (each returns a self-contained widget) ----
    QGroupBox* CreateInputFilesGroup();
    QGroupBox* CreateOutputGroup();
    QGroupBox* CreateOptimizationGroup();
    QGroupBox* CreateWarningGroup();
    QGroupBox* CreateLoggingGroup();
    QGroupBox* CreateCompilationTypeGroup();
    QWidget*   CreateLogPanel();
    QPushButton* CreateCompileButton();

    // ---- Input/Output widgets ----
    QListWidget* m_fileList      = nullptr;
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
};
