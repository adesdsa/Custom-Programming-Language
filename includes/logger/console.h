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

    // Create the only instance this class will have throughout the program
    // @return: Instance of the console class.
    static Console& GetInstance();

    // Build log message based on given params and log it.
    // @param entity: The entity that sent the message.
    // @param debugLevel: Type of message (INFO/Warnning/Error).
    // @param msg: The message content.
    void BuildAndSendLogMessage(const Entity& entity, const DebugLevel& debugLevel, const std::string msg);

private:
    // Private constructor and destructor so that no one could create instance of this class
    Console() = default;
    ~Console() = default;

    // Log message into console.
    // @param logMessage: Message to log with all the information built (entity sender, message type and the content).
    void Log(const LogMessage& logMessage);
};