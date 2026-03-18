#include "includes/driver/ui/gui/settings_panel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QPushButton>

SettingsPanel::SettingsPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 10, 0);
    layout->setSpacing(6);

    layout->addWidget(CreateInputFilesGroup());
    layout->addWidget(CreateOutputGroup());
    layout->addWidget(CreateOptimizationGroup());
    layout->addWidget(CreateWarningGroup());
    layout->addWidget(CreateLoggingGroup());
    layout->addWidget(CreateCompilationTypeGroup());
    layout->addStretch();
}

CompilerOptions SettingsPanel::BuildCompilerOptions() const
{
    CompilerOptions options;

    for (int i = 0; i < m_fileList->count(); ++i)
    {
        options.inputFilesPath.push_back(m_fileList->item(i)->text().toStdWString());
    }

    QString outputName = m_outputNameEdit->text().trimmed();
    if (!outputName.isEmpty())
    {
        options.outputFileName = outputName.toStdWString();
    }

    QString outputDir = m_outputDirEdit->text().trimmed();
    if (!outputDir.isEmpty())
    {
        options.outputDir = outputDir.toStdWString();
    }

    options.optimizationLevel = static_cast<short>(
        m_optimizationCombo->currentData().toInt()
    );

    options.warningLevel = static_cast<WarningLevel>(
        m_warningCombo->currentData().toInt()
    );

    options.isVerbose = m_verboseCheck->isChecked();
    options.isQuite   = m_quietCheck->isChecked();
    options.saveLogs  = m_saveLogsCheck->isChecked();

    options.stopAtAsm = m_stopAsmCheck->isChecked();
    options.stopAtBin = m_stopBinCheck->isChecked();
    options.saveTemps = m_saveTempsCheck->isChecked();

    return options;
}

int SettingsPanel::InputFileCount() const
{
    return m_fileList->count();
}

QString SettingsPanel::InputFileAt(int index) const
{
    return m_fileList->item(index)->text();
}

// =============================================================================
// Slots
// =============================================================================

void SettingsPanel::OnAddFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select picoc Source Files",
        QString(),
        "picoc files (*.picoc);;All files (*.*)"
    );

    for (const QString& file : files)
    {
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

void SettingsPanel::OnRemoveFiles()
{
    QList<QListWidgetItem*> selected = m_fileList->selectedItems();
    for (QListWidgetItem* item : selected)
    {
        delete item;
    }
}

void SettingsPanel::OnBrowseOutputDir()
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

QGroupBox* SettingsPanel::CreateInputFilesGroup()
{
    QGroupBox* group = new QGroupBox("Input Files (.picoc)");
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(8);

    m_fileList = new QListWidget();
    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileList->setMinimumHeight(90);
    layout->addWidget(m_fileList);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->setSpacing(8);
    QPushButton* addBtn    = new QPushButton("+ Add Files");
    QPushButton* removeBtn = new QPushButton("- Remove");
    addBtn->setCursor(Qt::PointingHandCursor);
    removeBtn->setCursor(Qt::PointingHandCursor);
    buttons->addWidget(addBtn);
    buttons->addWidget(removeBtn);
    buttons->addStretch();
    layout->addLayout(buttons);

    connect(addBtn,    &QPushButton::clicked, this, &SettingsPanel::OnAddFiles);
    connect(removeBtn, &QPushButton::clicked, this, &SettingsPanel::OnRemoveFiles);

    return group;
}

QGroupBox* SettingsPanel::CreateOutputGroup()
{
    QGroupBox* group = new QGroupBox("Output");
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(8);

    QHBoxLayout* nameRow = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Name:");
    nameLabel->setFixedWidth(70);
    nameRow->addWidget(nameLabel);
    m_outputNameEdit = new QLineEdit();
    m_outputNameEdit->setPlaceholderText("output");
    nameRow->addWidget(m_outputNameEdit);
    layout->addLayout(nameRow);

    QHBoxLayout* dirRow = new QHBoxLayout();
    QLabel* dirLabel = new QLabel("Directory:");
    dirLabel->setFixedWidth(70);
    dirRow->addWidget(dirLabel);
    m_outputDirEdit = new QLineEdit();
    m_outputDirEdit->setPlaceholderText("current working directory");
    dirRow->addWidget(m_outputDirEdit);

    QPushButton* browseBtn = new QPushButton("Browse...");
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QPushButton::clicked, this, &SettingsPanel::OnBrowseOutputDir);
    dirRow->addWidget(browseBtn);
    layout->addLayout(dirRow);

    return group;
}

QGroupBox* SettingsPanel::CreateOptimizationGroup()
{
    QGroupBox* group = new QGroupBox("Optimization");
    QHBoxLayout* layout = new QHBoxLayout(group);

    QLabel* label = new QLabel("Level:");
    label->setFixedWidth(50);
    layout->addWidget(label);
    m_optimizationCombo = new QComboBox();
    m_optimizationCombo->addItem("-O0  No optimizations", 0);
    m_optimizationCombo->addItem("-O1  Basic optimizations", 1);
    m_optimizationCombo->setCurrentIndex(1);
    layout->addWidget(m_optimizationCombo, 1);

    return group;
}

QGroupBox* SettingsPanel::CreateWarningGroup()
{
    QGroupBox* group = new QGroupBox("Warnings");
    QHBoxLayout* layout = new QHBoxLayout(group);

    QLabel* label = new QLabel("Level:");
    label->setFixedWidth(50);
    layout->addWidget(label);
    m_warningCombo = new QComboBox();
    m_warningCombo->addItem("Off   (--warn-off)", static_cast<int>(WarningLevel::OFF));
    m_warningCombo->addItem("Normal",             static_cast<int>(WarningLevel::NORMAL));
    m_warningCombo->addItem("All   (--warn-all)", static_cast<int>(WarningLevel::ALL));
    m_warningCombo->setCurrentIndex(1);
    layout->addWidget(m_warningCombo, 1);

    return group;
}

QGroupBox* SettingsPanel::CreateLoggingGroup()
{
    QGroupBox* group = new QGroupBox("Logging");
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(6);

    m_verboseCheck  = new QCheckBox("Verbose (-v)");
    m_quietCheck    = new QCheckBox("Quiet (-q)");
    m_saveLogsCheck = new QCheckBox("Save logs to file (--save-logs)");

    layout->addWidget(m_verboseCheck);
    layout->addWidget(m_quietCheck);
    layout->addWidget(m_saveLogsCheck);

    return group;
}

QGroupBox* SettingsPanel::CreateCompilationTypeGroup()
{
    QGroupBox* group = new QGroupBox("Pipeline Control");
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(6);

    m_stopAsmCheck   = new QCheckBox("Stop at assembly (--asm)");
    m_stopBinCheck   = new QCheckBox("Stop at object file (--bin)");
    m_saveTempsCheck = new QCheckBox("Keep intermediate files (--save-temp)");

    layout->addWidget(m_stopAsmCheck);
    layout->addWidget(m_stopBinCheck);
    layout->addWidget(m_saveTempsCheck);

    return group;
}

