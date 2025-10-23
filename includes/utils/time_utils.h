#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace TimeUtils
{
    // Get current time in format: dd/mm/yyyy hh/mm/ss.
    // @return: string tht hold current time in format: dd/mm/yyyy hh/mm/ss.
    std::string  GetCurrentTime();
}