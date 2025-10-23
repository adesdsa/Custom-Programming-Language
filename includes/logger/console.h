#pragma once

#include "logger_library.h"

// Responsible to log the messages
class Console
{
public:
    Console() = default;

    ~Console() = default;

    static LogMessage BuildAndSendLogMessage(const Entity& entity, const DebugLevel& debugLevel, const std::string msg);


private:
    void Log(LogMessage& logMessage);
};