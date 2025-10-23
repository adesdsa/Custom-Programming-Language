#pragma once

#include "logger_library.h"

#include <iostream>

// Will attach for each component in the compiler and will be his logger
class EntityLogger
{
public:
    EntityLogger(const std::string sEntityName);

    ~EntityLogger() = default;

    void Info(const std::string& msg);
    void Warnning(const std::string& msg);
    void Error(const std::string& msg);


private:
    Entity m_entity;
};