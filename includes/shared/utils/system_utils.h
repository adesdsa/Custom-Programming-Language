#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <locale>
#include <codecvt>

namespace SystemUtils
{
    // Get current time in format: dd/mm/yyyy hh/mm/ss.
    // @return: string that hold current time in format: dd/mm/yyyy hh/mm/ss.
    std::string GetFullCurrentTime();

    const std::filesystem::path BuildRelativePath(const std::wstring& relativePath);
}