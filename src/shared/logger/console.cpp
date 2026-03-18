#include "includes/shared/logger/console.h"
#include "includes/driver/compiler_options.h"

#include <iomanip>
#include <sstream>

Console::Console() :
    m_supportsColor(DetectColorSupport()),
    m_warningLevel(WarningLevel::NORMAL)
{}

Console& Console::GetInstance() noexcept
{
    static Console instance;  // created on first call
    return instance;
}

void Console::SetVerbosity(Verbosity level) noexcept
{
    m_verbosity = level;
}

Verbosity Console::GetVerbosity() const noexcept
{
    return m_verbosity;
}

void Console::SetWarningLevel(WarningLevel level) noexcept
{
    m_warningLevel = level;
}

WarningLevel Console::GetWarningLevel() const noexcept
{
    return m_warningLevel;
}

void Console::EnableFileLogging(const std::string& logFilePath) noexcept
{
    // close any previously open log file first
    if (this->m_logFile.is_open())
    {
        this->m_logFile.close();
    }

    this->m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
}

void Console::DisableFileLogging() noexcept
{
    if (this->m_logFile.is_open())
    {
        this->m_logFile.flush();
        this->m_logFile.close();
    }
}

void Console::SetLogCallback(LogCallback callback) noexcept
{
    this->m_logCallback = std::move(callback);
}

void Console::BuildAndSendLogMessage(
    const Entity &entity, 
    const DebugLevel &debugLevel, 
    const std::string msg
) const noexcept
{
    std::lock_guard<std::mutex> lock(this->m_consoleMutex);
    // --warn-off suppresses warnings everywhere
    if (debugLevel.sName == "WARN" && m_warningLevel == WarningLevel::OFF)
    {
        return;
    }

    LogMessage logMessage(entity, debugLevel, SystemUtils::GetFullCurrentTime(), msg);

    // always write to file when file logging is enabled
    if (this->m_logFile.is_open())
    {
        this->LogToFile(logMessage);
    }

    // only produce output when verbose
    if (this->m_verbosity == Verbosity::VERBOSE)
    {
        // stdout (CLI)
        this->Log(logMessage);

        // external callback (GUI log panel, etc.)
        if (this->m_logCallback)
        {
            const size_t k_nEntityW = 22;
            const size_t k_nLevelW  = 10;

            std::string entityBracketed = "[" + logMessage.entity.sName + "]";
            std::string levelBracketed  = "[" + logMessage.debugLevel.sName + "]";

            std::ostringstream oss;
            oss << logMessage.sTimeStamp << "\t"
                << std::left << std::setw(k_nEntityW) << entityBracketed
                << std::left << std::setw(k_nLevelW)  << levelBracketed
                << logMessage.sContent;

            this->m_logCallback(oss.str());
        }
    }
}

void Console::Log(const LogMessage& logMessage) const noexcept
{
    const std::string k_sEntityColor = this->m_supportsColor ? logMessage.entity.sColor : "";
    const std::string k_sLevelColor = this->m_supportsColor ? logMessage.debugLevel.sColor : "";

    const size_t k_nEntityWidth = 22;
    const size_t k_nLevelWidth  = 10;

    std::string entityBracketed = "[" + logMessage.entity.sName + "]";
    std::string levelBracketed = "[" + logMessage.debugLevel.sName + "]";

    const std::string& k_sContentColor =
        (logMessage.debugLevel.sName == "ERROR") ? k_sLevelColor : k_sEntityColor;

    std::cout
        << k_sEntityColor
        << logMessage.sTimeStamp << "\t"
        << std::left << std::setw(k_nEntityWidth) << entityBracketed
        << k_sReset
        << k_sLevelColor
        << std::left << std::setw(k_nLevelWidth) << levelBracketed
        << k_sReset
        << k_sContentColor
        << logMessage.sContent
        << k_sReset << std::endl;
}

void Console::LogToFile(const LogMessage& logMessage) const noexcept
{
    const size_t k_nEntityWidth = 22;
    const size_t k_nLevelWidth  = 10;

    std::string entityBracketed = "[" + logMessage.entity.sName + "]";
    std::string levelBracketed = "[" + logMessage.debugLevel.sName + "]";

    m_logFile
        << logMessage.sTimeStamp << "\t"
        << std::left << std::setw(k_nEntityWidth) << entityBracketed
        << std::left << std::setw(k_nLevelWidth) << levelBracketed
        << logMessage.sContent
        << std::endl; // flush each line so file is written immediately
}

bool Console::DetectColorSupport() noexcept
{
    #ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return false;

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) return false;

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode)) return false;

        return true;

    #else
        return isatty(fileno(stdout)) != 0; // Linux/macOS

    #endif
}
