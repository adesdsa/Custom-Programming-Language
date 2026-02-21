#include "includes/driver/ui/gui/main_window.h"
#include "includes/driver/compiler_options.h"
#include "includes/driver/options_validator.h"
#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QSplitter>
#include <QDir>
#include <QFrame>
#include <QScrollArea>
#include <QGridLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainter>
#include <QMouseEvent>
#include <stdexcept>
#include <filesystem>
#include <functional>

// =============================================================================
// Colour palette
// =============================================================================
static constexpr const char* COL_BG         = "#0d1117";
static constexpr const char* COL_SURFACE    = "#161b22";
static constexpr const char* COL_ELEVATED   = "#1c2333";
static constexpr const char* COL_INPUT      = "#0d1117";
static constexpr const char* COL_BORDER     = "#30363d";
static constexpr const char* COL_BORDER_LT  = "#484f58";
static constexpr const char* COL_TEXT       = "#e6edf3";
static constexpr const char* COL_MUTED      = "#7d8590";
static constexpr const char* COL_ACCENT     = "#7c3aed";
static constexpr const char* COL_ACCENT_LT  = "#a78bfa";
static constexpr const char* COL_CYAN       = "#22d3ee";
static constexpr const char* COL_GREEN      = "#22c55e";
static constexpr const char* COL_GREEN_LT   = "#4ade80";
static constexpr const char* COL_SELECTION  = "#1e3a5f";

// =============================================================================
// Inline HSV colour-picker widgets (no Q_OBJECT needed)
// =============================================================================
namespace {

/// @brief A square widget where X = hue (0-360), Y = saturation (255 → 0).
///        Painted at the current brightness value.
class HsvSquare : public QWidget
{
public:
    std::function<void()> onChanged;

    explicit HsvSquare(QWidget* p = nullptr) : QWidget(p)
    {
        setFixedSize(190, 190);
        setCursor(Qt::CrossCursor);
    }

    void setHsv(int h, int s, int v) { m_hue = h; m_sat = s; m_val = v; update(); }
    int hue() const { return m_hue; }
    int sat() const { return m_sat; }
    int val() const { return m_val; }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        QImage img(width(), height(), QImage::Format_RGB32);
        for (int y = 0; y < height(); ++y)
        {
            int s = 255 - y * 255 / (height() - 1);
            for (int x = 0; x < width(); ++x)
            {
                int h = x * 359 / (width() - 1);
                img.setPixelColor(x, y, QColor::fromHsv(h, s, m_val));
            }
        }
        p.drawImage(0, 0, img);
        // crosshair (plus shape)
        int cx = m_hue * (width() - 1) / 359;
        int cy = (255 - m_sat) * (height() - 1) / 255;
        constexpr int arm = 8;
        // black outline
        p.setPen(QPen(Qt::black, 3));
        p.drawLine(cx - arm, cy, cx + arm, cy);
        p.drawLine(cx, cy - arm, cx, cy + arm);
        // white cross on top
        p.setPen(QPen(Qt::white, 1.5));
        p.drawLine(cx - arm, cy, cx + arm, cy);
        p.drawLine(cx, cy - arm, cx, cy + arm);
    }

    void mousePressEvent(QMouseEvent* e) override { pick(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e)  override { pick(e->pos()); }

private:
    void pick(const QPoint& pos)
    {
        int x = qBound(0, pos.x(), width() - 1);
        int y = qBound(0, pos.y(), height() - 1);
        m_hue = x * 359 / (width() - 1);
        m_sat = 255 - y * 255 / (height() - 1);
        update();
        if (onChanged) onChanged();
    }
    int m_hue = 0, m_sat = 255, m_val = 255;
};

/// @brief A vertical bar widget for brightness (value): top = bright, bottom = dark.
class ValueBar : public QWidget
{
public:
    std::function<void()> onChanged;

    explicit ValueBar(QWidget* p = nullptr) : QWidget(p)
    {
        setFixedSize(22, 190);
        setCursor(Qt::PointingHandCursor);
    }

    void setHsv(int h, int s, int v) { m_hue = h; m_sat = s; m_val = v; update(); }
    int val() const { return m_val; }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        QImage img(width(), height(), QImage::Format_RGB32);
        for (int y = 0; y < height(); ++y)
        {
            int v = 255 - y * 255 / (height() - 1);
            QRgb rgb = QColor::fromHsv(m_hue, m_sat, v).rgb();
            for (int x = 0; x < width(); ++x)
                img.setPixel(x, y, rgb);
        }
        p.drawImage(0, 0, img);
        // arrow indicator
        int cy = (255 - m_val) * (height() - 1) / 255;
        p.setPen(QPen(Qt::white, 2));
        p.drawLine(0, cy, width(), cy);
    }

    void mousePressEvent(QMouseEvent* e) override { pick(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e)  override { pick(e->pos()); }

private:
    void pick(const QPoint& pos)
    {
        int y = qBound(0, pos.y(), height() - 1);
        m_val = 255 - y * 255 / (height() - 1);
        update();
        if (onChanged) onChanged();
    }
    int m_hue = 0, m_sat = 255, m_val = 255;
};

} // anonymous namespace

// =============================================================================
// Constructor
// =============================================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("picoc IDE");
    resize(1100, 750);
    setMinimumSize(850, 580);
    ApplyTheme();
    SetupUI();
}

// =============================================================================
// Theme
// =============================================================================

void MainWindow::ApplyTheme()
{
    setStyleSheet(GetStyleSheet());
}

QString MainWindow::GetStyleSheet()
{
    return QString(R"QSS(

    /* ================================================================ */
    /*  GLOBAL                                                          */
    /* ================================================================ */
    QMainWindow {
        background-color: %1;
    }
    QWidget {
        background-color: transparent;
        color: %2;
        font-family: "Segoe UI", "Inter", "Helvetica Neue", sans-serif;
        font-size: 10pt;
    }

    /* ================================================================ */
    /*  MENU BAR  (app-level title bar)                                 */
    /* ================================================================ */
    QWidget#appMenuBar {
        background-color: %3;
        border-bottom: 1px solid %4;
    }
    QPushButton#menuItem {
        background: transparent;
        color: %12;
        border: none;
        border-radius: 6px;
        padding: 6px 14px;
        font-size: 9.5pt;
        font-weight: 600;
        min-height: 18px;
    }
    QPushButton#menuItem:hover {
        background-color: %4;
        color: %2;
    }
    QPushButton#menuItem:pressed {
        background-color: %9;
    }

    /* ================================================================ */
    /*  GROUP BOXES  — coloured left accent bar                         */
    /* ================================================================ */
    QGroupBox {
        background-color: %3;
        border: 1px solid %4;
        border-left: 3px solid %5;
        border-radius: 10px;
        margin-top: 18px;
        padding: 18px 14px 12px 14px;
        font-weight: 600;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        subcontrol-position: top left;
        left: 14px;
        top: 2px;
        padding: 0 8px;
        color: %6;
        font-size: 10pt;
        font-weight: 700;
        background-color: %3;
        border-radius: 4px;
    }

    /* ================================================================ */
    /*  BUTTONS — general                                               */
    /* ================================================================ */
    QPushButton {
        background-color: %7;
        color: %2;
        border: 1px solid %4;
        border-radius: 8px;
        padding: 7px 18px;
        font-weight: 600;
        font-size: 9.5pt;
        min-height: 20px;
    }
    QPushButton:hover {
        background-color: %4;
        border-color: %9;
    }
    QPushButton:pressed {
        background-color: %9;
    }

    /* ---- compile button ---- */
    QPushButton#compileButton {
        background-color: %5;
        color: white;
        border: none;
        border-radius: 10px;
        font-size: 12pt;
        font-weight: 700;
        padding: 11px 0;
        letter-spacing: 0.5px;
    }
    QPushButton#compileButton:hover {
        background-color: %6;
    }
    QPushButton#compileButton:pressed {
        background-color: #6d28d9;
    }

    /* ================================================================ */
    /*  LINE EDITS                                                      */
    /* ================================================================ */
    QLineEdit {
        background-color: %13;
        color: %2;
        border: 1px solid %4;
        border-radius: 8px;
        padding: 7px 10px;
        font-size: 9.5pt;
        selection-background-color: %14;
    }
    QLineEdit:focus {
        border-color: %5;
    }
    QLineEdit::placeholder {
        color: %12;
    }

    /* ================================================================ */
    /*  COMBO BOXES                                                     */
    /* ================================================================ */
    QComboBox {
        background-color: %13;
        color: %2;
        border: 1px solid %4;
        border-radius: 8px;
        padding: 7px 32px 7px 10px;
        font-size: 9.5pt;
        min-height: 22px;
    }
    QComboBox:hover {
        border-color: %9;
    }
    QComboBox:focus {
        border-color: %5;
    }
    QComboBox::drop-down {
        subcontrol-origin: padding;
        subcontrol-position: center right;
        width: 28px;
        border: none;
    }
    QComboBox::down-arrow {
        image: url(:/icons/arrow-down.svg);
        width: 12px;
        height: 8px;
    }
    QComboBox QAbstractItemView {
        background-color: %7;
        color: %2;
        border: 1px solid %4;
        border-radius: 6px;
        selection-background-color: %5;
        selection-color: white;
        padding: 4px;
        outline: none;
    }

    /* ================================================================ */
    /*  CHECK BOXES                                                     */
    /* ================================================================ */
    QCheckBox {
        spacing: 10px;
        color: %2;
        font-size: 9.5pt;
        padding: 4px 0;
    }
    QCheckBox::indicator {
        width: 18px;
        height: 18px;
        border: 2px solid %9;
        border-radius: 5px;
        background-color: %7;
    }
    QCheckBox::indicator:hover {
        border-color: %6;
    }
    QCheckBox::indicator:checked {
        background-color: %5;
        border-color: %5;
        image: url(:/icons/check.svg);
    }
    QCheckBox::indicator:checked:hover {
        background-color: %6;
        border-color: %6;
    }

    /* ================================================================ */
    /*  LIST WIDGET                                                     */
    /* ================================================================ */
    QListWidget {
        background-color: %13;
        color: %2;
        border: 1px solid %4;
        border-radius: 8px;
        padding: 4px;
        font-size: 9pt;
    }
    QListWidget::item {
        padding: 5px 8px;
        border-radius: 6px;
    }
    QListWidget::item:selected {
        background-color: %14;
        color: %2;
    }
    QListWidget::item:hover {
        background-color: %7;
    }

    /* ================================================================ */
    /*  LOG TEXT AREA                                                    */
    /* ================================================================ */
    QTextEdit#logOutput {
        background-color: %13;
        color: %11;
        border: 1px solid %4;
        border-radius: 10px;
        padding: 10px;
        font-family: "Cascadia Code", "Fira Code", "Consolas", monospace;
        font-size: 9pt;
        selection-background-color: %14;
    }

    /* ================================================================ */
    /*  LABELS                                                          */
    /* ================================================================ */
    QLabel {
        color: %2;
        background: transparent;
    }
    QLabel#logHeading {
        color: %6;
        font-size: 10pt;
        font-weight: 700;
        padding-bottom: 4px;
    }

    /* ================================================================ */
    /*  STATUS BAR                                                      */
    /* ================================================================ */
    QWidget#statusBar {
        background-color: %3;
        border-top: 1px solid %4;
    }
    QLabel#statusVersion {
        color: %12;
        font-size: 8pt;
        font-weight: 600;
        padding: 0 4px;
    }

    /* ================================================================ */
    /*  SPLITTER                                                        */
    /* ================================================================ */
    QSplitter::handle {
        background: transparent;
        width: 6px;
    }

    /* ================================================================ */
    /*  SCROLLBARS                                                      */
    /* ================================================================ */
    QScrollBar:vertical {
        background: transparent;
        width: 8px;
    }
    QScrollBar::handle:vertical {
        background: %4;
        border-radius: 4px;
        min-height: 30px;
    }
    QScrollBar::handle:vertical:hover {
        background: %9;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
        background: none;
        height: 0;
    }
    QScrollBar:horizontal {
        background: transparent;
        height: 8px;
    }
    QScrollBar::handle:horizontal {
        background: %4;
        border-radius: 4px;
        min-width: 30px;
    }
    QScrollBar::handle:horizontal:hover {
        background: %9;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
    QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
        background: none;
        width: 0;
    }

    /* ================================================================ */
    /*  SEPARATOR                                                       */
    /* ================================================================ */
    QFrame#headerSeparator {
        max-height: 1px;
        border: none;
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:0,
            stop:0 transparent, stop:0.2 %5,
            stop:0.8 %15, stop:1 transparent);
    }

    /* ================================================================ */
    /*  SCROLL AREA                                                     */
    /* ================================================================ */
    QScrollArea {
        background: transparent;
        border: none;
    }
    /* settings panel scrollbar — only the handle is visible */
    QScrollArea#settingsScroll QScrollBar:vertical {
        width: 6px;
        background: transparent;
    }
    QScrollArea#settingsScroll QScrollBar::handle:vertical {
        background: %9;
        border-radius: 3px;
        min-height: 30px;
    }
    QScrollArea#settingsScroll QScrollBar::handle:vertical:hover {
        background: %12;
    }
    QScrollArea#settingsScroll QScrollBar::add-line:vertical,
    QScrollArea#settingsScroll QScrollBar::sub-line:vertical,
    QScrollArea#settingsScroll QScrollBar::add-page:vertical,
    QScrollArea#settingsScroll QScrollBar::sub-page:vertical {
        background: transparent;
        height: 0;
    }

    )QSS")
    .arg(COL_BG)         // %1
    .arg(COL_TEXT)        // %2
    .arg(COL_SURFACE)    // %3
    .arg(COL_BORDER)     // %4
    .arg(COL_ACCENT)     // %5  vivid purple
    .arg(COL_ACCENT_LT)  // %6  lighter purple
    .arg(COL_ELEVATED)   // %7
    .arg(COL_BORDER)     // %8  (unused placeholder)
    .arg(COL_BORDER_LT)  // %9  hover border
    .arg(COL_GREEN)      // %10 green
    .arg(COL_GREEN_LT)   // %11 light green (log text)
    .arg(COL_MUTED)      // %12 muted text
    .arg(COL_INPUT)      // %13 input bg
    .arg(COL_SELECTION)  // %14 selection bg
    .arg(COL_CYAN);      // %15 cyan
}

// =============================================================================
// UI Setup
// =============================================================================

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

    // left panel inside a scroll area so all groups are reachable
    QWidget* leftInner = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftInner);
    leftLayout->setContentsMargins(0, 0, 10, 0);   // right margin keeps scrollbar away from splitter
    leftLayout->setSpacing(6);

    leftLayout->addWidget(CreateInputFilesGroup());
    leftLayout->addWidget(CreateOutputGroup());
    leftLayout->addWidget(CreateOptimizationGroup());
    leftLayout->addWidget(CreateWarningGroup());
    leftLayout->addWidget(CreateLoggingGroup());
    leftLayout->addWidget(CreateCompilationTypeGroup());
    leftLayout->addStretch();

    QScrollArea* scroll = new QScrollArea();
    scroll->setObjectName("settingsScroll");
    scroll->setWidgetResizable(true);
    scroll->setWidget(leftInner);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    splitter->addWidget(scroll);
    splitter->addWidget(CreateLogPanel());
    splitter->setSizes({380, 600});
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    bodyLayout->addWidget(splitter, 1);

    // ---- compile button ----
    bodyLayout->addSpacing(4);
    bodyLayout->addWidget(CreateCompileButton());

    mainLayout->addWidget(body, 1);

    // ---- status bar (bottom) ----
    mainLayout->addWidget(CreateStatusBar());

    // ---- apply default terminal colours ----
    ApplyTerminalColors();
}

// =============================================================================
// Widget builders
// =============================================================================

QWidget* MainWindow::CreateMenuBar()
{
    QWidget* bar = new QWidget();
    bar->setObjectName("appMenuBar");
    bar->setFixedHeight(40);

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(12, 0, 12, 0);
    layout->setSpacing(4);

    // ---- logo ----
    QLabel* logoLabel = new QLabel();
    logoLabel->setText(
        QString("<span style='color: %1; font-size: 13pt; font-weight: 800;'>pic</span>"
                "<span style='color: %2; font-size: 13pt; font-weight: 800;'>oc</span>")
        .arg(COL_TEXT).arg(COL_CYAN)
    );
    logoLabel->setTextFormat(Qt::RichText);
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

QGroupBox* MainWindow::CreateInputFilesGroup()
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
    QPushButton* addBtn = new QPushButton("+ Add Files");
    QPushButton* removeBtn = new QPushButton("- Remove");
    addBtn->setCursor(Qt::PointingHandCursor);
    removeBtn->setCursor(Qt::PointingHandCursor);
    buttons->addWidget(addBtn);
    buttons->addWidget(removeBtn);
    buttons->addStretch();
    layout->addLayout(buttons);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::OnAddFiles);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::OnRemoveFiles);

    return group;
}

QGroupBox* MainWindow::CreateOutputGroup()
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
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::OnBrowseOutputDir);
    dirRow->addWidget(browseBtn);
    layout->addLayout(dirRow);

    return group;
}

QGroupBox* MainWindow::CreateOptimizationGroup()
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

QGroupBox* MainWindow::CreateWarningGroup()
{
    QGroupBox* group = new QGroupBox("Warnings");
    QHBoxLayout* layout = new QHBoxLayout(group);

    QLabel* label = new QLabel("Level:");
    label->setFixedWidth(50);
    layout->addWidget(label);
    m_warningCombo = new QComboBox();
    m_warningCombo->addItem("Off   (--warn-off)", static_cast<int>(WarningLevel::OFF));
    m_warningCombo->addItem("Normal", static_cast<int>(WarningLevel::NORMAL));
    m_warningCombo->addItem("All   (--warn-all)", static_cast<int>(WarningLevel::ALL));
    m_warningCombo->setCurrentIndex(1);
    layout->addWidget(m_warningCombo, 1);

    return group;
}

QGroupBox* MainWindow::CreateLoggingGroup()
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

QGroupBox* MainWindow::CreateCompilationTypeGroup()
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

QWidget* MainWindow::CreateLogPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(6, 0, 0, 0);

    QLabel* heading = new QLabel("Terminal Output");
    heading->setObjectName("logHeading");
    layout->addWidget(heading);

    m_logOutput = new QTextEdit();
    m_logOutput->setObjectName("logOutput");
    m_logOutput->setReadOnly(true);
    layout->addWidget(m_logOutput, 1);

    QHBoxLayout* buttons = new QHBoxLayout();
    QPushButton* clearBtn = new QPushButton("Clear");
    clearBtn->setCursor(Qt::PointingHandCursor);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::OnClearLog);
    buttons->addStretch();
    buttons->addWidget(clearBtn);
    layout->addLayout(buttons);

    return panel;
}

QPushButton* MainWindow::CreateCompileButton()
{
    m_compileButton = new QPushButton("Compile");
    m_compileButton->setObjectName("compileButton");
    m_compileButton->setMinimumHeight(48);
    m_compileButton->setCursor(Qt::PointingHandCursor);

    connect(m_compileButton, &QPushButton::clicked, this, &MainWindow::OnCompile);

    return m_compileButton;
}

// =============================================================================
// Slots
// =============================================================================

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

void MainWindow::OnShowUsage()
{
    const QString usage =
        "Usage: picoc [options] <file1.picoc> [file2.picoc ...]\n"
        "\n"
        "Options:\n"
        "\n"
        "  Meta\n"
        "    --help              Show this help message and exit\n"
        "    --version           Print compiler version and exit\n"
        "\n"
        "  Output\n"
        "    -o <file>           Set output executable name  (default: output)\n"
        "    --out-dir <dir>     Set output directory         (default: cwd)\n"
        "\n"
        "  Optimization\n"
        "    -O<level>           Set optimization level (0-1, default: 1)\n"
        "                          -O0  No optimizations\n"
        "                          -O1  Basic optimizations\n"
        "\n"
        "  Warnings\n"
        "    --warn-off          Disable all warnings\n"
        "    --warn-all          Enable all warnings including pedantic\n"
        "\n"
        "  Logging\n"
        "    -v                  Verbose output (print all compiler logs)\n"
        "    -q                  Quiet mode (suppress all logs)\n"
        "    --save-logs         Save logs to a file (<output>.log)\n"
        "\n"
        "  Pipeline Control\n"
        "    --asm               Stop after assembly generation (.asm)\n"
        "    --bin               Stop after assembler (.obj)\n"
        "    --save-temp         Keep all intermediate files\n"
        "\n"
        "Notes:\n"
        "  - All input files must have the .picoc extension.\n"
        "  - Each flag may only appear once.\n"
        "  - --help and --version are standalone (no other flags allowed).";

    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("picoc — Usage Reference");
    dialog->resize(680, 540);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setStyleSheet(GetStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(24, 20, 24, 18);

    QLabel* heading = new QLabel(
        QString("<span style='color: %1;'>Command-Line Reference</span>")
        .arg(COL_ACCENT_LT)
    );
    heading->setTextFormat(Qt::RichText);
    heading->setStyleSheet("font-size: 15pt; font-weight: 700;");
    layout->addWidget(heading);

    layout->addSpacing(8);

    QTextEdit* textArea = new QTextEdit();
    textArea->setObjectName("logOutput");
    textArea->setReadOnly(true);
    textArea->setPlainText(usage);
    textArea->setStyleSheet(
        QString("QTextEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  border: 1px solid %3;"
                "  border-radius: 10px;"
                "  padding: 10px;"
                "  font-family: 'Cascadia Code', 'Fira Code', 'Consolas', monospace;"
                "  font-size: 9pt;"
                "}")
        .arg(m_termBgColor.name())
        .arg(m_termTextColor.name())
        .arg(COL_BORDER)
    );
    layout->addWidget(textArea, 1);

    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    dialog->exec();
}

void MainWindow::OnCompile()
{
    AppendLog("========================================");
    AppendLog("Starting compilation...");
    AppendLog("========================================");

    // ---- Build CompilerOptions from UI ----
    CompilerOptions options;

    for (int i = 0; i < m_fileList->count(); ++i)
    {
        QString path = m_fileList->item(i)->text();
        options.inputFilesPath.push_back(path.toStdWString());
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

    // ---- Register log callback ----
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
    console.DisableFileLogging();
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

void MainWindow::ApplyTerminalColors()
{
    // Apply to the main log output and any future usage dialog text areas
    m_logOutput->setStyleSheet(
        QString("QTextEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  border: 1px solid %3;"
                "  border-radius: 10px;"
                "  padding: 10px;"
                "  font-family: 'Cascadia Code', 'Fira Code', 'Consolas', monospace;"
                "  font-size: 9pt;"
                "}")
        .arg(m_termBgColor.name())
        .arg(m_termTextColor.name())
        .arg(COL_BORDER)
    );
}

void MainWindow::OnShowTerminalSettings()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Terminal Appearance");
    dialog->setFixedSize(380, 280);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setStyleSheet(GetStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(24, 20, 24, 18);
    layout->setSpacing(14);

    QLabel* heading = new QLabel(
        QString("<span style='color: %1;'>Terminal Appearance</span>")
        .arg(COL_ACCENT_LT)
    );
    heading->setTextFormat(Qt::RichText);
    heading->setStyleSheet("font-size: 14pt; font-weight: 700;");
    layout->addWidget(heading);

    // ---- preview ----
    QTextEdit* preview = new QTextEdit();
    preview->setReadOnly(true);
    preview->setFixedHeight(80);
    preview->setPlainText("22/02/2026  [PARSER]  [INFO]  Ready\n"
                          "22/02/2026  [CODEGEN] [INFO]  Compiled OK");

    auto updatePreview = [=]() {
        preview->setStyleSheet(
            QString("QTextEdit {"
                    "  background-color: %1;"
                    "  color: %2;"
                    "  border: 1px solid %3;"
                    "  border-radius: 8px;"
                    "  padding: 8px;"
                    "  font-family: 'Cascadia Code', 'Fira Code', 'Consolas', monospace;"
                    "  font-size: 9pt;"
                    "}")
            .arg(m_termBgColor.name())
            .arg(m_termTextColor.name())
            .arg(COL_BORDER)
        );
    };
    updatePreview();
    layout->addWidget(preview);

    // ---- colour buttons ----
    QHBoxLayout* textRow = new QHBoxLayout();
    textRow->addWidget(new QLabel("Text Color:"));
    QPushButton* textColorBtn = new QPushButton();
    textColorBtn->setFixedSize(60, 28);
    textColorBtn->setCursor(Qt::PointingHandCursor);
    textColorBtn->setStyleSheet(
        QString("background-color: %1; border: 1px solid %2; border-radius: 6px;")
        .arg(m_termTextColor.name()).arg(COL_BORDER_LT)
    );
    textRow->addWidget(textColorBtn);
    textRow->addStretch();
    layout->addLayout(textRow);

    QHBoxLayout* bgRow = new QHBoxLayout();
    bgRow->addWidget(new QLabel("Background:"));
    QPushButton* bgColorBtn = new QPushButton();
    bgColorBtn->setFixedSize(60, 28);
    bgColorBtn->setCursor(Qt::PointingHandCursor);
    bgColorBtn->setStyleSheet(
        QString("background-color: %1; border: 1px solid %2; border-radius: 6px;")
        .arg(m_termBgColor.name()).arg(COL_BORDER_LT)
    );
    bgRow->addWidget(bgColorBtn);
    bgRow->addStretch();
    layout->addLayout(bgRow);

    // ---- helper: custom colour-picker dialog (no QColorDialog hacks) ----
    // Colours are sorted by hue column so shades sit together.
    auto openColorPicker = [](const QColor& initial, QWidget* parent, const QString& title) -> QColor {

        // ---- Preset palette: 9 hue columns x 6 lightness rows (sorted by hue) ----
        static const char* palette[6][9] = {
            // Red        Orange      Yellow      Lime        Green       Cyan        Blue        Purple      Pink
            { "#ffcccc", "#ffe0cc", "#ffffcc", "#e0ffcc", "#ccffcc", "#ccffe0", "#ccccff", "#e0ccff", "#ffccff" },
            { "#ff9999", "#ffc299", "#ffff99", "#c2ff99", "#99ff99", "#99ffc2", "#9999ff", "#c299ff", "#ff99ff" },
            { "#ff4d4d", "#ff944d", "#ffff4d", "#94ff4d", "#4dff4d", "#4dff94", "#4d4dff", "#944dff", "#ff4dff" },
            { "#ff0000", "#ff6600", "#ffff00", "#66ff00", "#00ff00", "#00ff66", "#0000ff", "#6600ff", "#ff00ff" },
            { "#cc0000", "#cc5200", "#cccc00", "#52cc00", "#00cc00", "#00cc52", "#0000cc", "#5200cc", "#cc00cc" },
            { "#800000", "#803300", "#808000", "#338000", "#008000", "#008033", "#000080", "#330080", "#800080" },
        };
        static const char* neutrals[] = {
            "#ffffff", "#e0e0e0", "#c0c0c0", "#a0a0a0",
            "#808080", "#606060", "#404040", "#202020", "#000000"
        };

        QColor chosen = initial;

        QDialog dlg(parent);
        dlg.setWindowTitle(title);
        dlg.setFixedWidth(640);
        dlg.setStyleSheet(
            QString(
                "QDialog { background-color: %1; }"
                "QLabel  { color: %2; font-size: 9pt; }"
                "QPushButton#colorCell:hover { border-color: #ffffff !important; }"
                "QLineEdit {"
                "   background-color: %3; color: %2; border: 1px solid %4;"
                "   border-radius: 6px; padding: 5px 10px;"
                "   font-family: 'Cascadia Code', 'Consolas', monospace; font-size: 10pt;"
                "}"
                "QPushButton#dlgBtn {"
                "   background-color: %5; color: %2; border: none;"
                "   border-radius: 6px; padding: 7px 24px; font-weight: 600; font-size: 9.5pt;"
                "}"
                "QPushButton#dlgBtn:hover { background-color: %6; }"
            )
            .arg(COL_SURFACE)
            .arg(COL_TEXT)
            .arg(COL_INPUT)
            .arg(COL_BORDER)
            .arg(COL_ACCENT)
            .arg(COL_ACCENT_LT)
        );

        QVBoxLayout* mainLay = new QVBoxLayout(&dlg);
        mainLay->setContentsMargins(20, 16, 20, 16);
        mainLay->setSpacing(14);

        // ---- Preview swatch + hex input (shared by both pickers) ----
        QLabel* swatch = new QLabel();
        swatch->setFixedSize(52, 52);
        swatch->setStyleSheet(
            QString("background-color: %1; border: 2px solid %2; border-radius: 10px;")
            .arg(chosen.name()).arg(COL_BORDER_LT)
        );

        QLineEdit* hexEdit = new QLineEdit(chosen.name());
        hexEdit->setMaxLength(7);
        hexEdit->setValidator(new QRegularExpressionValidator(
            QRegularExpression("^#[0-9A-Fa-f]{0,6}$"), &dlg));

        // ---- HSV picker widgets ----
        HsvSquare* hsvSquare = new HsvSquare();
        ValueBar*  valBar    = new ValueBar();

        // Initialise HSV widgets from the initial colour
        int ih, is, iv;
        initial.getHsv(&ih, &is, &iv);
        if (ih < 0) ih = 0;
        hsvSquare->setHsv(ih, is, iv);
        valBar->setHsv(ih, is, iv);

        // ---- Shared update helper ----
        auto updateAll = [&](const QColor& c) {
            chosen = c;
            swatch->setStyleSheet(
                QString("background-color: %1; border: 2px solid %2; border-radius: 10px;")
                .arg(c.name()).arg(COL_BORDER_LT)
            );
            hexEdit->blockSignals(true);
            hexEdit->setText(c.name());
            hexEdit->blockSignals(false);
        };

        auto syncFromHsv = [&]() {
            QColor c = QColor::fromHsv(hsvSquare->hue(), hsvSquare->sat(), valBar->val());
            valBar->setHsv(hsvSquare->hue(), hsvSquare->sat(), valBar->val());
            hsvSquare->setHsv(hsvSquare->hue(), hsvSquare->sat(), valBar->val());
            updateAll(c);
        };

        hsvSquare->onChanged = syncFromHsv;
        valBar->onChanged    = syncFromHsv;

        // ---- Top section: preset grid (left) + HSV picker (right) ----
        constexpr int CELL = 26;
        constexpr int GAP  = 6;

        QHBoxLayout* topRow = new QHBoxLayout();
        topRow->setSpacing(24);

        // -- Left: preset colour grid inside a tight wrapper --
        QWidget* presetWidget = new QWidget();
        presetWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        QVBoxLayout* presetCol = new QVBoxLayout(presetWidget);
        presetCol->setContentsMargins(0, 0, 0, 0);
        presetCol->setSpacing(6);

        QLabel* presetLabel = new QLabel("Presets");
        presetLabel->setStyleSheet("font-weight: 700; font-size: 9.5pt;");
        presetCol->addWidget(presetLabel);

        QGridLayout* grid = new QGridLayout();
        grid->setSpacing(GAP);
        grid->setSizeConstraint(QLayout::SetFixedSize);

        const QString cellQss =
            QString("min-width: %1px; max-width: %1px; min-height: %1px; max-height: %1px;"
                    " border: 2px solid transparent; border-radius: 4px; padding: 0; margin: 0;")
            .arg(CELL);

        for (int row = 0; row < 6; ++row)
        {
            for (int col = 0; col < 9; ++col)
            {
                QPushButton* cell = new QPushButton();
                cell->setObjectName("colorCell");
                cell->setFixedSize(CELL, CELL);
                cell->setCursor(Qt::PointingHandCursor);
                QColor cc(palette[row][col]);
                cell->setStyleSheet(
                    QString("background-color: %1; %2").arg(cc.name(), cellQss)
                );
                QObject::connect(cell, &QPushButton::clicked, [cc, &updateAll, hsvSquare, valBar]() {
                    updateAll(cc);
                    int h, s, v;
                    cc.getHsv(&h, &s, &v);
                    if (h < 0) h = 0;
                    hsvSquare->setHsv(h, s, v);
                    valBar->setHsv(h, s, v);
                });
                grid->addWidget(cell, row, col);
            }
        }
        // neutrals row
        for (int col = 0; col < 9; ++col)
        {
            QPushButton* cell = new QPushButton();
            cell->setObjectName("colorCell");
            cell->setFixedSize(CELL, CELL);
            cell->setCursor(Qt::PointingHandCursor);
            QColor cc(neutrals[col]);
            cell->setStyleSheet(
                QString("background-color: %1; %2").arg(cc.name(), cellQss)
            );
            QObject::connect(cell, &QPushButton::clicked, [cc, &updateAll, hsvSquare, valBar]() {
                updateAll(cc);
                int h, s, v;
                cc.getHsv(&h, &s, &v);
                if (h < 0) h = 0;
                hsvSquare->setHsv(h, s, v);
                valBar->setHsv(h, s, v);
            });
            grid->addWidget(cell, 6, col);
        }
        presetCol->addLayout(grid);
        topRow->addWidget(presetWidget, 0, Qt::AlignTop);

        // -- Right: HSV gradient picker --
        QWidget* hsvWidget = new QWidget();
        QVBoxLayout* hsvCol = new QVBoxLayout(hsvWidget);
        hsvCol->setContentsMargins(0, 0, 0, 0);
        hsvCol->setSpacing(6);

        QLabel* customLabel = new QLabel("Custom");
        customLabel->setStyleSheet("font-weight: 700; font-size: 9.5pt;");
        hsvCol->addWidget(customLabel);

        QHBoxLayout* hsvRow = new QHBoxLayout();
        hsvRow->setSpacing(8);
        hsvRow->addWidget(hsvSquare);
        hsvRow->addWidget(valBar);
        hsvCol->addLayout(hsvRow);
        topRow->addWidget(hsvWidget, 0, Qt::AlignTop);

        topRow->addStretch();
        mainLay->addLayout(topRow);

        // ---- hex + swatch row ----
        QHBoxLayout* hexRow = new QHBoxLayout();
        hexRow->setSpacing(12);

        QLabel* hexLabel = new QLabel("HTML:");
        hexLabel->setStyleSheet("font-weight: 600;");
        hexRow->addWidget(hexLabel);
        hexRow->addWidget(hexEdit, 1);
        hexRow->addWidget(swatch);
        mainLay->addLayout(hexRow);

        QObject::connect(hexEdit, &QLineEdit::textChanged, [&](const QString& text) {
            if (text.length() == 7)
            {
                QColor c(text);
                if (c.isValid())
                {
                    chosen = c;
                    swatch->setStyleSheet(
                        QString("background-color: %1; border: 2px solid %2; border-radius: 10px;")
                        .arg(c.name()).arg(COL_BORDER_LT)
                    );
                    int h, s, v;
                    c.getHsv(&h, &s, &v);
                    if (h < 0) h = 0;
                    hsvSquare->setHsv(h, s, v);
                    valBar->setHsv(h, s, v);
                }
            }
        });

        // ---- OK / Cancel ----
        QHBoxLayout* btnRow = new QHBoxLayout();
        btnRow->addStretch();

        QPushButton* okBtn = new QPushButton("OK");
        okBtn->setObjectName("dlgBtn");
        okBtn->setCursor(Qt::PointingHandCursor);
        QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
        btnRow->addWidget(okBtn);

        QPushButton* cancelBtn = new QPushButton("Cancel");
        cancelBtn->setObjectName("dlgBtn");
        cancelBtn->setCursor(Qt::PointingHandCursor);
        QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
        btnRow->addWidget(cancelBtn);
        mainLay->addLayout(btnRow);

        return dlg.exec() == QDialog::Accepted ? chosen : QColor();
    };

    // ---- colour picker connections ----
    connect(textColorBtn, &QPushButton::clicked, this, [=]() {
        QColor c = openColorPicker(m_termTextColor, dialog, "Choose Text Color");
        if (c.isValid())
        {
            m_termTextColor = c;
            textColorBtn->setStyleSheet(
                QString("background-color: %1; border: 1px solid %2; border-radius: 6px;")
                .arg(c.name()).arg(COL_BORDER_LT)
            );
            updatePreview();
        }
    });

    connect(bgColorBtn, &QPushButton::clicked, this, [=]() {
        QColor c = openColorPicker(m_termBgColor, dialog, "Choose Background Color");
        if (c.isValid())
        {
            m_termBgColor = c;
            bgColorBtn->setStyleSheet(
                QString("background-color: %1; border: 1px solid %2; border-radius: 6px;")
                .arg(c.name()).arg(COL_BORDER_LT)
            );
            updatePreview();
        }
    });

    // ---- buttons ----
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton* applyBtn = new QPushButton("Apply");
    applyBtn->setCursor(Qt::PointingHandCursor);
    connect(applyBtn, &QPushButton::clicked, this, [=]() {
        ApplyTerminalColors();
        dialog->accept();
    });
    btnRow->addWidget(applyBtn);

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    layout->addLayout(btnRow);

    dialog->exec();
}
