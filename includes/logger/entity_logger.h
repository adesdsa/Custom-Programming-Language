#pragma once

#include "console.h"

// Will attach for each component in the compiler and will be his logger
class EntityLogger
{
public:
    EntityLogger(const std::string& sEntityName);
    
    EntityLogger(const EntityLogger& other);

    ~EntityLogger() = default;

    void Info(const std::string& msg) const;

    void Warnning(const std::string& msg) const;

    void Error(const std::string& msg) const;

private:
    Entity m_entity;
    Console& m_console;
};