#pragma once

#include "includes/driver/ui/gui/settings_panel.h"
#include "includes/driver/ui/gui/log_panel.h"
#include "includes/driver/ui/gui/compilation_handler.h"

#include <QMainWindow>
#include <QPushButton>

/// @brief Top-level window that composes SettingsPanel, LogPanel,
///        and CompilationHandler.  All domain logic and widget details
///        live in the sub-components; this class only wires them together.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void OnCompile();
    void OnShowUsage();
    void OnShowTerminalSettings();

private:
    void SetupUI();

    QWidget*     CreateMenuBar();
    QWidget*     CreateStatusBar();
    QPushButton* CreateCompileButton();

    // ---- sub-components ----
    SettingsPanel*      m_settingsPanel      = nullptr;
    LogPanel*           m_logPanel           = nullptr;
    CompilationHandler* m_compilationHandler = nullptr;
    QPushButton*        m_compileButton      = nullptr;
};
