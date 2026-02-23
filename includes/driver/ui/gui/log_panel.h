#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QColor>

/// @brief Widget that displays compilation log output and manages terminal
///        appearance settings (text colour, background colour).
class LogPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget* parent = nullptr);
    ~LogPanel() override = default;

    /// @brief Append a single message to the log.
    void AppendLog(const QString& message);

    /// @brief Current terminal text colour.
    QColor TermTextColor() const;

    /// @brief Current terminal background colour.
    QColor TermBgColor() const;

public slots:
    /// @brief Clear all log content.
    void Clear();

    /// @brief Apply the stored terminal colours to the log output widget.
    void ApplyTerminalColors();

    /// @brief Show the terminal-appearance settings dialog and apply changes.
    void ShowTerminalSettings();

    /// @brief Set the terminal text and background colours directly.
    void SetTerminalColors(const QColor& textColor, const QColor& bgColor);

private:
    QTextEdit* m_logOutput     = nullptr;
    QColor m_termTextColor     = QColor("#4ade80");   // default green
    QColor m_termBgColor       = QColor("#0d1117");   // default dark
};

