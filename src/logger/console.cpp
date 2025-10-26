#include "../../includes/logger/console.h"

#include "../../includes/utils/time_utils.h"

Console& Console::GetInstance()
{
    static Console instance; // Created on first call
    return instance;
}

void Console::BuildAndSendLogMessage(const Entity &entity, const DebugLevel &debugLevel, const std::string msg)
{
    this->Log(LogMessage(entity, debugLevel, TimeUtils::GetCurrentTime(), msg));
}

void Console::Log(const LogMessage& logMessage)
{
    // Widths for padding **outside** brackets
    const int entityWidth = 12;
    const int levelWidth  = 8;

    std::cout 
        << logMessage.entity.sColor
        << logMessage.sTimeStamp << "\t"
        << "[" << logMessage.entity.sName << "]"
        << std::setw(entityWidth - logMessage.entity.sName.size()) << "" // padding outside brackets
        << k_sReset
        << logMessage.debugLevel.sColor
        << "[" << logMessage.debugLevel.sName << "]"
        << std::setw(levelWidth - logMessage.debugLevel.sName.size()) << "" // padding outside brackets
        << k_sReset << "\t"
        << logMessage.entity.sColor
        << logMessage.sContent
        << k_sReset << std::endl;
}