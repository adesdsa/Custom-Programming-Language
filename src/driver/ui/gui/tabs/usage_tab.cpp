#include "includes/driver/ui/gui/tabs/usage_tab.h"
#include "includes/driver/ui/gui/theme.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>

using namespace Theme;

void RenderUsageTab(QWidget* parent, const QColor& termBgColor, const QColor& termTextColor)
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

    QDialog* dialog = new QDialog(parent);
    dialog->setWindowTitle("picoc — Usage Reference");
    dialog->resize(680, 540);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setStyleSheet(Theme::GetStyleSheet());

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
        .arg(termBgColor.name())
        .arg(termTextColor.name())
        .arg(COL_BORDER)
    );
    layout->addWidget(textArea, 1);

    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setCursor(Qt::PointingHandCursor);
    QObject::connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    dialog->exec();
}
