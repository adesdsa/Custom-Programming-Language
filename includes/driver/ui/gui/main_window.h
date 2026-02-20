#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>

/// @brief Main window for the picoc GUI IDE.
///
/// Provides a graphical interface for selecting source files,
/// configuring compiler options, and viewing compilation output.
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
    /// @brief Build the UI layout.
    void SetupUI();

    /// @brief Append a message to the log output area.
    void AppendLog(const QString& message);

    // ---- Input/Output widgets ----
    QListWidget* m_fileList;
    QLineEdit*   m_outputNameEdit;
    QLineEdit*   m_outputDirEdit;

    // ---- Flag widgets ----
    QComboBox* m_optimizationCombo;
    QComboBox* m_warningCombo;
    QCheckBox* m_verboseCheck;
    QCheckBox* m_quietCheck;
    QCheckBox* m_saveLogsCheck;
    QCheckBox* m_stopAsmCheck;
    QCheckBox* m_stopBinCheck;
    QCheckBox* m_saveTempsCheck;

    // ---- Action widgets ----
    QPushButton* m_compileButton;
    QTextEdit*   m_logOutput;
};
