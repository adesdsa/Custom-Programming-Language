#include "includes/driver/ui/gui/log_panel.h"
#include "includes/driver/ui/gui/tabs/terminal_tab.h"
#include "includes/driver/ui/gui/theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QPushButton>

using namespace Theme;

LogPanel::LogPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 0, 0, 0);

    QLabel* heading = new QLabel("Terminal Output");
    heading->setObjectName("logHeading");
    layout->addWidget(heading);

    this->m_logOutput = new QTextEdit();
    this->m_logOutput->setObjectName("logOutput");
    this->m_logOutput->setReadOnly(true);
    layout->addWidget(this->m_logOutput, 1);

    QHBoxLayout* buttons = new QHBoxLayout();
    QPushButton* clearBtn = new QPushButton("Clear");
    clearBtn->setCursor(Qt::PointingHandCursor);
    connect(clearBtn, &QPushButton::clicked, this, &LogPanel::Clear);
    buttons->addStretch();
    buttons->addWidget(clearBtn);
    layout->addLayout(buttons);
}

void LogPanel::AppendLog(const QString& message)
{
    this->m_logOutput->append(message);
}

QColor LogPanel::TermTextColor() const
{
    return this->m_termTextColor;
}

QColor LogPanel::TermBgColor() const
{
    return this->m_termBgColor;
}

void LogPanel::Clear()
{
    this->m_logOutput->clear();
}

void LogPanel::ApplyTerminalColors()
{
    this->m_logOutput->setStyleSheet(
        QString("QTextEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  border: 1px solid %3;"
                "  border-radius: 10px;"
                "  padding: 10px;"
                "  font-family: 'Cascadia Code', 'Fira Code', 'Consolas', monospace;"
                "  font-size: 9pt;"
                "}")
        .arg(this->m_termBgColor.name())
        .arg(this->m_termTextColor.name())
        .arg(COL_BORDER)
    );
}

void LogPanel::ShowTerminalSettings()
{
    auto result = RenderTerminalTab(this, this->m_termTextColor, this->m_termBgColor);
    if (result)
    {
        this->m_termTextColor = result->first;
        this->m_termBgColor   = result->second;
        ApplyTerminalColors();
    }
}

void LogPanel::SetTerminalColors(const QColor& textColor, const QColor& bgColor)
{
    this->m_termTextColor = textColor;
    this->m_termBgColor   = bgColor;
    ApplyTerminalColors();
}
