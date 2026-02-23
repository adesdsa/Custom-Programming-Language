#pragma once

#include "includes/driver/compiler_options.h"

#include <QWidget>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>

/// @brief Widget that contains every compiler-setting control.
///        Exposes BuildCompilerOptions() so callers can read the current UI state.
class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget* parent = nullptr);
    ~SettingsPanel() override = default;

    /// @brief Read the current UI state and return a populated CompilerOptions.
    CompilerOptions BuildCompilerOptions() const;

    /// @brief Number of input files currently listed.
    int InputFileCount() const;

    /// @brief Display text for input file at the given row.
    QString InputFileAt(int index) const;

private slots:
    void OnAddFiles();
    void OnRemoveFiles();
    void OnBrowseOutputDir();

private:
    QGroupBox* CreateInputFilesGroup();
    QGroupBox* CreateOutputGroup();
    QGroupBox* CreateOptimizationGroup();
    QGroupBox* CreateWarningGroup();
    QGroupBox* CreateLoggingGroup();
    QGroupBox* CreateCompilationTypeGroup();

    // ---- widgets ----
    QListWidget* m_fileList       = nullptr;
    QLineEdit*   m_outputNameEdit = nullptr;
    QLineEdit*   m_outputDirEdit  = nullptr;
    QComboBox*   m_optimizationCombo = nullptr;
    QComboBox*   m_warningCombo      = nullptr;
    QCheckBox*   m_verboseCheck      = nullptr;
    QCheckBox*   m_quietCheck        = nullptr;
    QCheckBox*   m_saveLogsCheck     = nullptr;
    QCheckBox*   m_stopAsmCheck      = nullptr;
    QCheckBox*   m_stopBinCheck      = nullptr;
    QCheckBox*   m_saveTempsCheck    = nullptr;
};

