#include "includes/driver/ui/gui/main_window.h"
#include "includes/driver/compiler_options.h"
#include "includes/driver/options_validator.h"
#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QSplitter>
#include <QDir>
#include <stdexcept>
#include <filesystem>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("picoc IDE");
    resize(900, 650);
    SetupUI();
}

void MainWindow::SetupUI()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ---- title ----
    QLabel* titleLabel = new QLabel("picoc Compiler");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // ---- left panel (settings) | right panel (log) ----
    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    leftLayout->addWidget(CreateInputFilesGroup());
    leftLayout->addWidget(CreateOutputGroup());
    leftLayout->addWidget(CreateOptimizationGroup());
    leftLayout->addWidget(CreateWarningGroup());
    leftLayout->addWidget(CreateLoggingGroup());
    leftLayout->addWidget(CreateCompilationTypeGroup());
    leftLayout->addStretch();

    splitter->addWidget(leftPanel);
    splitter->addWidget(CreateLogPanel());
    splitter->setSizes({350, 550});

    mainLayout->addWidget(splitter, 1);

    // ---- compile button ----
    mainLayout->addWidget(CreateCompileButton());
}

QGroupBox* MainWindow::CreateInputFilesGroup()
{
    QGroupBox* group = new QGroupBox("Input Files (.picoc)");
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_fileList = new QListWidget();
    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(m_fileList);

    QHBoxLayout* buttons = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton("Add Files...");
    QPushButton* removeBtn = new QPushButton("Remove Selected");
    buttons->addWidget(addBtn);
    buttons->addWidget(removeBtn);
    layout->addLayout(buttons);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::OnAddFiles);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::OnRemoveFiles);

    return group;
}

QGroupBox* MainWindow::CreateOutputGroup()
{
    QGroupBox* group = new QGroupBox("Output");
    QVBoxLayout* layout = new QVBoxLayout(group);

    // name row
    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->addWidget(new QLabel("Name:"));
    m_outputNameEdit = new QLineEdit();
    m_outputNameEdit->setPlaceholderText("output (default)");
    nameRow->addWidget(m_outputNameEdit);
    layout->addLayout(nameRow);

    // directory row
    QHBoxLayout* dirRow = new QHBoxLayout();
    dirRow->addWidget(new QLabel("Directory:"));
    m_outputDirEdit = new QLineEdit();
    m_outputDirEdit->setPlaceholderText("current working directory (default)");
    dirRow->addWidget(m_outputDirEdit);

    QPushButton* browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::OnBrowseOutputDir);
    dirRow->addWidget(browseBtn);
    layout->addLayout(dirRow);

    return group;
}

QGroupBox* MainWindow::CreateOptimizationGroup()
{
    QGroupBox* group = new QGroupBox("Optimization");
    QHBoxLayout* layout = new QHBoxLayout(group);

    layout->addWidget(new QLabel("Level:"));
    m_optimizationCombo = new QComboBox();
    m_optimizationCombo->addItem("-O0  (No optimizations)", 0);
    m_optimizationCombo->addItem("-O1  (Basic optimizations)", 1);
    m_optimizationCombo->setCurrentIndex(1);
    layout->addWidget(m_optimizationCombo);

    return group;
}

QGroupBox* MainWindow::CreateWarningGroup()
{
    QGroupBox* group = new QGroupBox("Warnings");
    QHBoxLayout* layout = new QHBoxLayout(group);

    layout->addWidget(new QLabel("Level:"));
    m_warningCombo = new QComboBox();
    m_warningCombo->addItem("Off (--warn-off)", static_cast<int>(WarningLevel::OFF));
    m_warningCombo->addItem("Normal (default)", static_cast<int>(WarningLevel::NORMAL));
    m_warningCombo->addItem("All (--warn-all)", static_cast<int>(WarningLevel::ALL));
    m_warningCombo->setCurrentIndex(1);
    layout->addWidget(m_warningCombo);

    return group;
}

QGroupBox* MainWindow::CreateLoggingGroup()
{
    QGroupBox* group = new QGroupBox("Logging");
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_verboseCheck  = new QCheckBox("Verbose (-v)");
    m_quietCheck    = new QCheckBox("Quiet (-q)");
    m_saveLogsCheck = new QCheckBox("Save logs to file (--save-logs)");

    layout->addWidget(m_verboseCheck);
    layout->addWidget(m_quietCheck);
    layout->addWidget(m_saveLogsCheck);

    return group;
}

QGroupBox* MainWindow::CreateCompilationTypeGroup()
{
    QGroupBox* group = new QGroupBox("Compilation Output");
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_stopAsmCheck   = new QCheckBox("Stop at assembly (--asm)");
    m_stopBinCheck   = new QCheckBox("Stop at object file (--bin)");
    m_saveTempsCheck = new QCheckBox("Save intermediate files (--save-temp)");

    layout->addWidget(m_stopAsmCheck);
    layout->addWidget(m_stopBinCheck);
    layout->addWidget(m_saveTempsCheck);

    return group;
}

QWidget* MainWindow::CreateLogPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* heading = new QLabel("Compilation Output");
    QFont headingFont = heading->font();
    headingFont.setBold(true);
    heading->setFont(headingFont);
    layout->addWidget(heading);

    m_logOutput = new QTextEdit();
    m_logOutput->setReadOnly(true);
    m_logOutput->setFont(QFont("Consolas", 9));
    layout->addWidget(m_logOutput, 1);

    QHBoxLayout* buttons = new QHBoxLayout();
    QPushButton* clearBtn = new QPushButton("Clear Log");
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::OnClearLog);
    buttons->addStretch();
    buttons->addWidget(clearBtn);
    layout->addLayout(buttons);

    return panel;
}

QPushButton* MainWindow::CreateCompileButton()
{
    m_compileButton = new QPushButton("Compile");
    m_compileButton->setMinimumHeight(40);

    QFont btnFont = m_compileButton->font();
    btnFont.setPointSize(12);
    btnFont.setBold(true);
    m_compileButton->setFont(btnFont);

    connect(m_compileButton, &QPushButton::clicked, this, &MainWindow::OnCompile);

    return m_compileButton;
}

// =============================================
// Slots
// =============================================

void MainWindow::OnAddFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select picoc Source Files",
        QString(),
        "picoc files (*.picoc);;All files (*.*)"
    );

    for (const QString& file : files)
    {
        // avoid duplicates
        bool exists = false;
        for (int i = 0; i < m_fileList->count(); ++i)
        {
            if (m_fileList->item(i)->text() == file)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            m_fileList->addItem(file);
        }
    }
}

void MainWindow::OnRemoveFiles()
{
    QList<QListWidgetItem*> selected = m_fileList->selectedItems();
    for (QListWidgetItem* item : selected)
    {
        delete item;
    }
}

void MainWindow::OnBrowseOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Output Directory",
        m_outputDirEdit->text().isEmpty()
            ? QDir::currentPath()
            : m_outputDirEdit->text()
    );

    if (!dir.isEmpty())
    {
        m_outputDirEdit->setText(dir);
    }
}

void MainWindow::OnCompile()
{
    AppendLog("========================================");
    AppendLog("Starting compilation...");
    AppendLog("========================================");

    // ---- Build CompilerOptions from UI ----
    CompilerOptions options;

    // Input files
    for (int i = 0; i < m_fileList->count(); ++i)
    {
        QString path = m_fileList->item(i)->text();
        options.inputFilesPath.push_back(path.toStdWString());
    }

    // Output name
    QString outputName = m_outputNameEdit->text().trimmed();
    if (!outputName.isEmpty())
    {
        options.outputFileName = outputName.toStdWString();
    }

    // Output directory
    QString outputDir = m_outputDirEdit->text().trimmed();
    if (!outputDir.isEmpty())
    {
        options.outputDir = outputDir.toStdWString();
    }

    // Optimization level
    options.optimizationLevel = static_cast<short>(
        m_optimizationCombo->currentData().toInt()
    );

    // Warning level
    options.warningLevel = static_cast<WarningLevel>(
        m_warningCombo->currentData().toInt()
    );

    // Logging flags
    options.isVerbose = m_verboseCheck->isChecked();
    options.isQuite = m_quietCheck->isChecked();
    options.saveLogs = m_saveLogsCheck->isChecked();

    // Compilation type flags
    options.stopAtAsm = m_stopAsmCheck->isChecked();
    options.stopAtBin = m_stopBinCheck->isChecked();
    options.saveTemps = m_saveTempsCheck->isChecked();

    // ---- Validate ----
    try
    {
        OptionsValidator::Validate(options);
    }
    catch (const std::runtime_error& e)
    {
        QMessageBox::warning(this, "Validation Error", QString::fromStdString(e.what()));
        AppendLog("Validation FAILED: " + QString::fromStdString(e.what()));
        return;
    }

    // ---- Log the options ----
    AppendLog("Input files:");
    for (int i = 0; i < m_fileList->count(); ++i)
    {
        AppendLog("  " + m_fileList->item(i)->text());
    }
    std::filesystem::path outputBase = options.outputDir.empty()
        ? std::filesystem::current_path()
        : std::filesystem::path(options.outputDir);
    std::filesystem::path outputFullPath = outputBase / options.outputFileName;
    AppendLog("Output: " + QString::fromStdWString(outputFullPath.wstring()));
    AppendLog("Optimization: -O" + QString::number(options.optimizationLevel));

    // ---- Register log callback so Console logs appear in the GUI ----
    // The callback fires independently of -v / -q (stdout verbosity).
    // The orchestrator will set verbosity based on the user's choice.
    Console& console = Console::GetInstance();
    console.SetLogCallback([this](const std::string& line)
    {
        AppendLog(QString::fromStdString(line));
    });

    // ---- Compile ----
    try
    {
        CompilerOrchestrator orchestrator(std::move(options));
        orchestrator.MainFlow();
        AppendLog("========================================");
        AppendLog("Compilation SUCCEEDED");
        AppendLog("========================================");
    }
    catch (const std::exception& e)
    {
        AppendLog("========================================");
        AppendLog("Compilation FAILED: " + QString::fromStdString(e.what()));
        AppendLog("========================================");
        QMessageBox::critical(this, "Compilation Error", QString::fromStdString(e.what()));
    }
    catch (...)
    {
        AppendLog("Compilation FAILED: Unknown error");
        QMessageBox::critical(this, "Compilation Error", "An unknown error occurred.");
    }

    // ---- Cleanup ----
    console.DisableFileLogging(); // flush & close log file
    console.SetLogCallback(nullptr);
}

void MainWindow::OnClearLog()
{
    m_logOutput->clear();
}

void MainWindow::AppendLog(const QString& message)
{
    m_logOutput->append(message);
}
