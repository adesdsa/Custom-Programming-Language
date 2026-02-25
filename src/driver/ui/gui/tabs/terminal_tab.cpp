#include "includes/driver/ui/gui/tabs/terminal_tab.h"
#include "includes/driver/ui/gui/theme.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>

using namespace Theme;

std::optional<std::pair<QColor, QColor>> RenderTerminalTab(
    QWidget* parent,
    const QColor& currentTextColor,
    const QColor& currentBgColor)
{
    // work on local copies so originals are not modified if the user cancels
    QColor textColor = currentTextColor;
    QColor bgColor   = currentBgColor;

    QDialog dialog(parent);
    dialog.setWindowTitle("Terminal Appearance");
    dialog.setFixedSize(380, 280);
    dialog.setStyleSheet(Theme::GetStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
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
    preview->setPlainText(
        "22/02/2026  [PARSER]  [INFO]  Ready\n"
        "22/02/2026  [CODEGEN] [INFO]  Compiled OK"
    );

    auto updatePreview = [&]() {
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
            .arg(bgColor.name())
            .arg(textColor.name())
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
        .arg(textColor.name()).arg(COL_BORDER_LT)
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
        .arg(bgColor.name()).arg(COL_BORDER_LT)
    );
    bgRow->addWidget(bgColorBtn);
    bgRow->addStretch();
    layout->addLayout(bgRow);

    // ---- color picker connections ----
    QObject::connect(textColorBtn, &QPushButton::clicked, &dialog, [&]() {
        QColor color = OpenColorPicker(textColor, &dialog, "Choose Text Color");
        if (color.isValid())
        {
            textColor = color;
            textColorBtn->setStyleSheet(
                QString("background-color: %1; border: 1px solid %2; border-radius: 6px;")
                .arg(color.name()).arg(COL_BORDER_LT)
            );
            updatePreview();
        }
    });

    QObject::connect(bgColorBtn, &QPushButton::clicked, &dialog, [&]() {
        QColor color = OpenColorPicker(bgColor, &dialog, "Choose Background Color");
        if (color.isValid())
        {
            bgColor = color;
            bgColorBtn->setStyleSheet(
                QString("background-color: %1; border: 1px solid %2; border-radius: 6px;")
                .arg(color.name()).arg(COL_BORDER_LT)
            );
            updatePreview();
        }
    });

    // ---- buttons ----
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton* applyBtn = new QPushButton("Apply");
    applyBtn->setCursor(Qt::PointingHandCursor);
    QObject::connect(applyBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    btnRow->addWidget(applyBtn);

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    layout->addLayout(btnRow);

    if (dialog.exec() == QDialog::Accepted)
    {
        return std::make_pair(textColor, bgColor);
    }

    return std::nullopt;
}
