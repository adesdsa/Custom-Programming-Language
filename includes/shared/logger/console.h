#pragma once

#include <iostream>
#include <fstream>

#include "logger_library.h"
#include "includes/shared/utils/system_utils.h"

// forward declaration (defined in compiler_options.h)
enum class WarningLevel : int;

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <cstdio>
#endif

/// Controls how much logging output is produced.
enum class Verbosity
{
    QUIET,    // suppress all logs (default; errors still shown via ErrorManager)
    VERBOSE   // print all compiler logs at UI (-v)
};

// Responsible to log the messages
class Console
{
public:
    // Delete copy/move to prevent duplicates
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    // Create the only instance this class will have throughout the program
    // @return: Instance of the console class.
    static Console& GetInstance() noexcept;

    /// @brief Set the global verbosity level (call before pipeline starts).
    void SetVerbosity(Verbosity level) noexcept;

    /// @brief Get the current verbosity level.
    Verbosity GetVerbosity() const noexcept;

    /// @brief Set the warning level.
    void SetWarningLevel(WarningLevel level) noexcept;

    /// @brief Get the current warning level.
    WarningLevel GetWarningLevel() const noexcept;

    /// @brief Enable file logging — all logs are also written to <outputName>.log alongside output files.
    void EnableFileLogging(const std::string& outputName) noexcept;

    // Build log message based on given params and log it.
    // @param entity: The entity that sent the message.
    // @param debugLevel: Type of message (INFO/Warnning/Error).
    // @param msg: The message content.
    void BuildAndSendLogMessage(
        const Entity& entity, 
        const DebugLevel& debugLevel, 
        const std::string msg
    ) const noexcept;

private:
    bool m_supportsColor;
    Verbosity m_verbosity = Verbosity::QUIET;
    WarningLevel m_warningLevel;
    mutable std::ofstream m_logFile;

    // Private constructor and destructor so that no one could create instance of this class
    Console();
    ~Console() = default;

    /// @brief Log message into console.
    void Log(const LogMessage& logMessage) const noexcept;

    /// @brief Log message into file.
    void LogToFile(const LogMessage& logMessage) const noexcept;

    // Detects whether the console supports colored text output, and enables if it doesn't.
    static bool DetectColorSupport() noexcept;
};