#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <array>
#include <cstdio>

namespace SystemUtils
{
    // Get current time in format: dd/mm/yyyy hh/mm/ss.
    // @return: string that hold current time in format: dd/mm/yyyy hh/mm/ss.
    std::string GetFullCurrentTime();

    const std::filesystem::path BuildRelativePath(const std::wstring& relativePath);

    /// @brief Execute an external command and capture its stdout + stderr.
    /// @param command The shell command to execute (stderr is redirected to stdout).
    /// @param output  Receives the combined stdout + stderr text.
    /// @return The process exit code (0 = success).
    int RunProcess(const std::string& command, std::string& output);
}