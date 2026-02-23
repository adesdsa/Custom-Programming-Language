#pragma once

#include <QWidget>
#include <QColor>

#include <optional>
#include <utility>

/// @brief Shows the terminal-appearance settings dialog (text colour, background colour).
/// @param parent           Parent widget for the dialog.
/// @param currentTextColor The current terminal text colour.
/// @param currentBgColor   The current terminal background colour.
/// @return A pair (textColor, bgColor) if the user clicked Apply, or std::nullopt if cancelled.
std::optional<std::pair<QColor, QColor>> RenderTerminalTab(
    QWidget* parent,
    const QColor& currentTextColor,
    const QColor& currentBgColor);

