#pragma once

#include <iostream>

#include "logger_library.h"

// Responsible to log the messages
class Console
{
public:
    // Delete copy/move to prevent duplicates
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    // Access the single instance
    static Console& GetInstance();

    void BuildAndSendLogMessage(const Entity& entity, const DebugLevel& debugLevel, const std::string msg);

private:
    Console() = default;
    ~Console() = default;

    void Log(const LogMessage& logMessage);
};