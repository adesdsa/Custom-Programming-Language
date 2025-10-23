#include "../../includes/logger/console.h"

#include "../../includes/utils/time_utils.h"

LogMessage Console::BuildAndSendLogMessage(const Entity &entity, const DebugLevel &debugLevel, const std::string msg)
{
    return LogMessage(entity, debugLevel, TimeUtils::GetCurrentTime(), msg);
}

void Console::Log(LogMessage &logMessage)
{
    std::cout << logMessage.sTimeStamp << "\t" <<
    "[" << logMessage.entity << "]" << "\t" <<
    "[" << logMessage.debugLevel << "]" << "\t" <<
    logMessage.sContent << std::endl;
}