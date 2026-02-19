#include "includes/shared/logger/console.h"

Console::Console()
{
    static int x = 0;
    this->m_supportsColor = DetectColorSupport();
}

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

void Console::BuildAndSendLogMessage(
    const Entity &entity, 
    const DebugLevel &debugLevel, 
    const std::string msg
) const noexcept
{
    if (m_verbosity == Verbosity::QUIET)
    {
        return;
    }

    this->Log(
        LogMessage(entity, debugLevel, SystemUtils::GetFullCurrentTime(), msg)
    );
}

void Console::Log(const LogMessage& logMessage) const noexcept
{
    const std::string k_sEntityColor = this->m_supportsColor ? logMessage.entity.sColor : "";
    const std::string k_sLevelColor = this->m_supportsColor ? logMessage.debugLevel.sColor : "";

    const size_t k_nEntityWidth = 22;
    const size_t k_nLevelWidth  = 10;

    // Build bracketed strings
    std::string entityBracketed = "[" + logMessage.entity.sName + "]";
    std::string levelBracketed = "[" + logMessage.debugLevel.sName + "]";

    std::cout
        << k_sEntityColor
        << logMessage.sTimeStamp << "\t"
        << std::left << std::setw(k_nEntityWidth) << entityBracketed
        << k_sReset
        << k_sLevelColor
        << std::left << std::setw(k_nLevelWidth) << levelBracketed
        << k_sReset
        << k_sEntityColor
        << logMessage.sContent
        << k_sReset << std::endl;
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
