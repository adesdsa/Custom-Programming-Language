#include "../../includes/logger/console.h"

#include "../../includes/utils/time_utils.h"

LogMessage Console::BuildAndSendLogMessage(const Entity &entity, const DebugLevel &debugLevel, const std::string msg)
{
    return LogMessage(entity, debugLevel, TimeUtils::GetCurrentTime(), msg);
}

void Console::Log(LogMessage &logMessage)
{
    std::cout 
        << logMessage.entity.sColor                      // Entity color
        << logMessage.sTimeStamp << "\t"                 // Timestamp
        << "[" << logMessage.entity.sName << "]" << "\t" // Entity name
        << k_sReset                                      // Reset Entity color
        << logMessage.debugLevel.sColor                  // Level color
        << "[" << logMessage.debugLevel.sName << "]"     // Level name (string, not enum)
        << RESET << "\t"                                 // Reset level color
        << logMessage.entity.sColor                      // Restore entity color
        << logMessage.sContent                           // Log message content
        << RESET << std::endl;                           // Reset & Flush message
}