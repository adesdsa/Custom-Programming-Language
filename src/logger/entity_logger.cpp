#include "logger/entity_logger.h"

EntityLogger::EntityLogger(const std::string& sEntityName) :
    m_entity(sEntityName, ENTITY_COLORS.at(sEntityName)),
    m_console(Console::GetInstance()) {}

EntityLogger::EntityLogger(const EntityLogger& other) : 
    m_entity(other.m_entity), 
    m_console(other.m_console) {}

void EntityLogger::Info(const std::string &msg) const
{
    this->m_console.BuildAndSendLogMessage(
        this->m_entity, DebugLevel("INFO", DEBUG_COLORS.at("info")), msg
    );
}

void EntityLogger::Warnning(const std::string &msg) const
{
    this->m_console.BuildAndSendLogMessage(
        this->m_entity, DebugLevel("WARN", DEBUG_COLORS.at("warn")), msg
    );
}

void EntityLogger::Error(const std::string &msg) const
{
    this->m_console.BuildAndSendLogMessage(
        this->m_entity, DebugLevel("ERROR", DEBUG_COLORS.at("error")), msg
    );
}
