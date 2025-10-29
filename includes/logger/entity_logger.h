#pragma once

#include "console.h"

// Will attach for each component in the compiler and will be his logger
class EntityLogger
{
public:
    EntityLogger(const std::string& sEntityName);

    ~EntityLogger() = default;

    // Log info message.
    // @param msg: The content of the message.
    void Info(const std::string& msg);

    // Log warnning message.
    // @param msg: The content of the message.
    void Warnning(const std::string& msg);

    // Log error message.
    // @param msg: The content of the message.
    void Error(const std::string& msg);


private:
    Entity m_entity;
    Console& m_console;
};