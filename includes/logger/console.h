#pragma once

#include "logger_library.h"

// Responsible to log the messages
class Console
{
public:
    Console() = default;

    ~Console() = default;

    void Log(LogMessage& logMessage);
};