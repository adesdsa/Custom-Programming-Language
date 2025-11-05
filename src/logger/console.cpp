#include "../../includes/logger/console.h"


Console::Console() 
{
    this->m_supportsColor = DetectColorSupport();
}

Console& Console::GetInstance()
{
    static Console instance; // Created on first call
    return instance;
}

void Console::BuildAndSendLogMessage(const Entity &entity, const DebugLevel &debugLevel, const std::string msg)
{
    this->Log(LogMessage(entity, debugLevel, TimeUtils::GetFullCurrentTime(), msg));
}

bool Console::DetectColorSupport()
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

void Console::Log(const LogMessage& logMessage)
{
    const std::string entityColor = this->m_supportsColor ? logMessage.entity.sColor : "";
    const std::string levelColor = this->m_supportsColor ? logMessage.debugLevel.sColor : "";
    const int entityWidth = 12;
    const int levelWidth  = 8;

    std::cout
        << entityColor
        << logMessage.sTimeStamp << "\t"
        << "[" << logMessage.entity.sName << "]"
        << std::setw(entityWidth - logMessage.entity.sName.size()) << ""
        << k_sReset
        << levelColor
        << "[" << logMessage.debugLevel.sName << "]"
        << std::setw(levelWidth - logMessage.debugLevel.sName.size()) << ""
        << k_sReset << "\t"
        << entityColor
        << logMessage.sContent
        << k_sReset << std::endl;
}