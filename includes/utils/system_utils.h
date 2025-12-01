#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace SystemUtils
{
    // Get current time in format: dd/mm/yyyy hh/mm/ss.
    // @return: string that hold current time in format: dd/mm/yyyy hh/mm/ss.
    std::string GetFullCurrentTime();

    std::string BuildRelativePath(const std::string& relativePath);
}