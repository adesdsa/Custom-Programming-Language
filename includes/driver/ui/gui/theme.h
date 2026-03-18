#pragma once

#include <QString>
#include <QColor>
#include <QWidget>

/// @brief Centralised colour palette & stylesheet for the picoc GUI.
namespace Theme
{
    // ---- colour palette ----
    constexpr const char* COL_BG         = "#0d1117";
    constexpr const char* COL_SURFACE    = "#161b22";
    constexpr const char* COL_ELEVATED   = "#1c2333";
    constexpr const char* COL_INPUT      = "#0d1117";
    constexpr const char* COL_BORDER     = "#30363d";
    constexpr const char* COL_BORDER_LT  = "#484f58";
    constexpr const char* COL_TEXT       = "#e6edf3";
    constexpr const char* COL_MUTED      = "#7d8590";
    constexpr const char* COL_ACCENT     = "#7c3aed";
    constexpr const char* COL_ACCENT_LT  = "#a78bfa";
    constexpr const char* COL_CYAN       = "#22d3ee";
    constexpr const char* COL_GREEN      = "#22c55e";
    constexpr const char* COL_GREEN_LT   = "#4ade80";
    constexpr const char* COL_SELECTION  = "#1e3a5f";

    /// @brief Return the full QSS stylesheet for the application.
    QString GetStyleSheet();
}

/// @brief Open a custom colour-picker dialog with presets and an HSV gradient.
/// @return The chosen colour, or an invalid QColor if the user cancelled.
QColor OpenColorPicker(const QColor& initial, QWidget* parent, const QString& title);

