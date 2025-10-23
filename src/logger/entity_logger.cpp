#include "../../includes/logger/entity_logger.h"

#include "../../includes/logger/console.h"
#include "entity_logger.h"

EntityLogger::EntityLogger(const std::string sEntityName) : 
m_entity(sEntityName, ENTITY_COLORS.at(sEntityName))

void EntityLogger::Info(const std::string &msg)
{
    Console::BuildAndSendLogMessage(this->m_entity, DEBUG_COLORS.at("info"), msg);
}

void EntityLogger::Warnning(const std::string &msg)
{
    Console::BuildAndSendLogMessage(this->m_entity, DEBUG_COLORS.at("warn"), msg);

}

void EntityLogger::Error(const std::string &msg)
{
    Console::BuildAndSendLogMessage(this->m_entity, DEBUG_COLORS.at("error"), msg);
}
