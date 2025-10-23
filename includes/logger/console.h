#pragma once

#include "logger_library.h"

// Responsible to log the messages
class Console
{
public:
    Console() = default;

    ~Console() = default;

    LogMessage BuildAndSendLogMessage(const Entity& entity, const DebugLevel& debugLevel, const std::string msg);

    void Log(LogMessage& logMessage);
};