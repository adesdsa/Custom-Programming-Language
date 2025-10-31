#pragma once

#include "console.h"

// Will attach for each component in the compiler and will be his logger
class EntityLogger
{
public:
    EntityLogger(const std::string& sEntityName);

    EntityLogger(const EntityLogger& other);

    ~EntityLogger() = default;

    // Log info message.
    // @param msg: The content of the message.
    void Info(const std::string& msg) const;

    // Log warnning message.
    // @param msg: The content of the message.
    void Warnning(const std::string& msg) const;

    // Log error message.
    // @param msg: The content of the message.
    void Error(const std::string& msg) const;


private:
    Entity m_entity;
    Console& m_console;
};