#include "../../includes/logger/entity_logger.h"

#include "../../includes/logger/console.h"

EntityLogger::EntityLogger(const std::string sEntityName) : 
    m_entity(sEntityName, ENTITY_COLORS.at(sEntityName)), 
    m_console(Console::GetInstance()) {}

void EntityLogger::Info(const std::string &msg)
{
    this->m_console.BuildAndSendLogMessage(
        this->m_entity, DebugLevel("INFO", DEBUG_COLORS.at("info")), msg
    );
}

void EntityLogger::Warnning(const std::string &msg)
{
    this->m_console.BuildAndSendLogMessage(
        this->m_entity, DebugLevel("WARN", DEBUG_COLORS.at("warn")), msg
    );

}

void EntityLogger::Error(const std::string &msg)
{
    this->m_console.BuildAndSendLogMessage(
        this->m_entity, DebugLevel("ERROR", DEBUG_COLORS.at("error")), msg
    );
}
