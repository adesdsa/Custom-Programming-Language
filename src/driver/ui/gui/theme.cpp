#include "includes/driver/ui/gui/theme.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <functional>

using namespace Theme;

QString Theme::GetStyleSheet()
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
        background-color: %7;
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
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
            stop:0 #7c3aed, stop:0.5 #a855f7, stop:1 #22d3ee);
        color: white;
        border: 2px solid rgba(167, 139, 250, 0.4);
        border-radius: 14px;
        font-size: 11pt;
        font-weight: 700;
        padding: 10px 48px;
        letter-spacing: 1.5px;
        min-width: 180px;
        max-width: 260px;
    }
    QPushButton#compileButton:hover {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
            stop:0 #a78bfa, stop:0.5 #c084fc, stop:1 #67e8f9);
        border-color: rgba(196, 181, 253, 0.7);
    }
    QPushButton#compileButton:pressed {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
            stop:0 #6d28d9, stop:0.5 #7c3aed, stop:1 #06b6d4);
        border-color: rgba(124, 58, 237, 0.6);
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
// Inline HSV colour-picker widgets
// =============================================================================
namespace {

    /// @brief A square widget where X = hue (0-360), Y = saturation (255 → 0).
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
            int cx = m_hue * (width() - 1) / 359;
            int cy = (255 - m_sat) * (height() - 1) / 255;
            constexpr int arm = 8;
            p.setPen(QPen(Qt::black, 3));
            p.drawLine(cx - arm, cy, cx + arm, cy);
            p.drawLine(cx, cy - arm, cx, cy + arm);
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

    /// @brief A vertical bar widget for brightness (value).
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
}

QColor OpenColorPicker(const QColor& initial, QWidget* parent, const QString& title)
{
    // ---- Preset palette: 9 hue columns x 6 lightness rows ----
    static const char* palette[6][9] = {
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

    // ---- Preview swatch + hex input ----
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

    // -- Left: preset colour grid --
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

    QLabel* hexLabel = new QLabel("HEX:");
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
}

