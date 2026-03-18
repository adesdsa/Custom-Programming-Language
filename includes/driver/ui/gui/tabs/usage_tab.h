#pragma once

#include <QWidget>
#include <QColor>

/// @brief Show the picoc command-line usage reference dialog.
/// @param parent       Parent widget for the dialog.
/// @param termBgColor  Terminal background colour (used for the text area).
/// @param termTextColor Terminal text colour (used for the text area).
void RenderUsageTab(QWidget* parent, const QColor& termBgColor, const QColor& termTextColor);
