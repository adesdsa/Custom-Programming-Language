#include "logger/console.h"


Console::Console()
{
    this->m_supportsColor = DetectColorSupport();
}

Console& Console::GetInstance() noexcept
{
    static Console instance;  // created on first call
    return instance;
}

void Console::BuildAndSendLogMessage(
    const Entity &entity, 
    const DebugLevel &debugLevel, 
    const std::string msg
) const noexcept
{
    this->Log(
        LogMessage(entity, debugLevel, TimeUtils::GetFullCurrentTime(), msg)
    );
}

void Console::Log(const LogMessage& logMessage) const noexcept
{
    const std::string k_sEntityColor = this->m_supportsColor ? logMessage.entity.sColor : "";
    const std::string k_sLevelColor = this->m_supportsColor ? logMessage.debugLevel.sColor : "";
    const int k_nEntityWidth = 12;
    const int k_nLevelWidth  = 8;

    std::cout
        << k_sEntityColor
        << logMessage.sTimeStamp << "\t"
        << "[" << logMessage.entity.sName << "]"
        << std::setw(k_nEntityWidth - logMessage.entity.sName.size()) << ""
        << k_sReset
        << k_sLevelColor
        << "[" << logMessage.debugLevel.sName << "]"
        << std::setw(k_nLevelWidth - logMessage.debugLevel.sName.size()) << ""
        << k_sReset << "\t"
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
