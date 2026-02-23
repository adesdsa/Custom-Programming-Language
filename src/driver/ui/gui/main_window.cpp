#include "includes/driver/ui/gui/main_window.h"
#include "includes/driver/ui/gui/theme.h"
#include "includes/driver/ui/gui/tabs/usage_tab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QSplitter>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>
#include <QSizePolicy>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>

using namespace Theme;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("picoc IDE");
    resize(1100, 750);
    setMinimumSize(850, 580);
    setStyleSheet(Theme::GetStyleSheet());
    SetupUI();
}

void MainWindow::SetupUI()
{
    QWidget* central = new QWidget(this);
    central->setStyleSheet(QString("background-color: %1;").arg(COL_BG));
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ---- menu bar (top) ----
    mainLayout->addWidget(CreateMenuBar());

    // ---- body ----
    QWidget* body = new QWidget();
    QVBoxLayout* bodyLayout = new QVBoxLayout(body);
    bodyLayout->setSpacing(10);
    bodyLayout->setContentsMargins(18, 12, 18, 12);

    QFrame* sep = new QFrame();
    sep->setObjectName("headerSeparator");
    sep->setFrameShape(QFrame::HLine);
    sep->setFixedHeight(2);
    bodyLayout->addWidget(sep);

    bodyLayout->addSpacing(2);

    // ---- splitter: settings | log ----
    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    m_settingsPanel = new SettingsPanel();
    m_logPanel      = new LogPanel();

    QScrollArea* scroll = new QScrollArea();
    scroll->setObjectName("settingsScroll");
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_settingsPanel);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    splitter->addWidget(scroll);
    splitter->addWidget(m_logPanel);
    splitter->setSizes({380, 600});
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    bodyLayout->addWidget(splitter, 1);

    // ---- compile button (centered) ----
    bodyLayout->addSpacing(4);
    QHBoxLayout* compileRow = new QHBoxLayout();
    compileRow->addStretch();
    compileRow->addWidget(CreateCompileButton());
    compileRow->addStretch();
    bodyLayout->addLayout(compileRow);

    mainLayout->addWidget(body, 1);

    // ---- status bar (bottom) ----
    mainLayout->addWidget(CreateStatusBar());

    // ---- apply default terminal colours ----
    m_logPanel->ApplyTerminalColors();

    // ---- compilation handler (business logic, no UI) ----
    m_compilationHandler = new CompilationHandler(this);

    connect(m_compilationHandler, &CompilationHandler::LogMessage,
            m_logPanel,           &LogPanel::AppendLog);

    connect(m_compilationHandler, &CompilationHandler::ValidationFailed,
            this, [this](const QString& error)
    {
        QMessageBox::warning(this, "Validation Error", error);
    });

    connect(m_compilationHandler, &CompilationHandler::CompilationFailed,
            this, [this](const QString& error)
    {
        QMessageBox::critical(this, "Compilation Error", error);
    });
}

QWidget* MainWindow::CreateMenuBar()
{
    QWidget* bar = new QWidget();
    bar->setObjectName("appMenuBar");
    bar->setFixedHeight(60);

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(12, 0, 12, 0);
    layout->setSpacing(4);

    // ---- logo ----
    QLabel* logoLabel = new QLabel();
    logoLabel->setStyleSheet("background: transparent; border: none; padding: 0;");
    QPixmap logoPix(":/icons/logo.png");
    logoLabel->setPixmap(logoPix.scaledToHeight(52, Qt::SmoothTransformation));

    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(logoLabel);
    glow->setBlurRadius(24);
    glow->setOffset(0, 0);
    glow->setColor(QColor(255, 255, 255, 160));
    logoLabel->setGraphicsEffect(glow);

    layout->addWidget(logoLabel);

    layout->addSpacing(16);

    // ---- menu items ----
    QPushButton* usageBtn = new QPushButton("Usage");
    usageBtn->setObjectName("menuItem");
    usageBtn->setCursor(Qt::PointingHandCursor);
    connect(usageBtn, &QPushButton::clicked, this, &MainWindow::OnShowUsage);
    layout->addWidget(usageBtn);

    QPushButton* termBtn = new QPushButton("Terminal");
    termBtn->setObjectName("menuItem");
    termBtn->setCursor(Qt::PointingHandCursor);
    connect(termBtn, &QPushButton::clicked, this, &MainWindow::OnShowTerminalSettings);
    layout->addWidget(termBtn);

    layout->addStretch();

    return bar;
}

QWidget* MainWindow::CreateStatusBar()
{
    QWidget* bar = new QWidget();
    bar->setObjectName("statusBar");
    bar->setFixedHeight(26);

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(14, 0, 14, 0);

    layout->addStretch();

    QLabel* versionLabel = new QLabel(
        QString("<span style='color: %1;'>picoc</span>"
                " <span style='color: %2;'>v1.0.0</span>")
        .arg(COL_TEXT).arg(COL_MUTED)
    );
    versionLabel->setObjectName("statusVersion");
    versionLabel->setTextFormat(Qt::RichText);
    layout->addWidget(versionLabel);

    return bar;
}

QPushButton* MainWindow::CreateCompileButton()
{
    m_compileButton = new QPushButton("▶  Compile");
    m_compileButton->setObjectName("compileButton");
    m_compileButton->setMinimumHeight(44);
    m_compileButton->setCursor(Qt::PointingHandCursor);
    m_compileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(m_compileButton, &QPushButton::clicked, this, &MainWindow::OnCompile);

    return m_compileButton;
}

void MainWindow::OnCompile()
{
    CompilerOptions options = m_settingsPanel->BuildCompilerOptions();
    m_compilationHandler->Compile(std::move(options));
}

void MainWindow::OnShowUsage()
{
    RenderUsageTab(this, m_logPanel->TermBgColor(), m_logPanel->TermTextColor());
}

void MainWindow::OnShowTerminalSettings()
{
    m_logPanel->ShowTerminalSettings();
}
