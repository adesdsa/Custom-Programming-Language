#pragma once

#include <string>

struct Entity
{
    std::string sName;
    std::string sColor;
};


class EntityLogger
{
public:
    EntityLogger() = default;

    ~EntityLogger() = default;

    void Info(std::string msg);
    void Warnning(std::string msg);
    void Error(std::string msg);


private:
    // Debugger& m_pDebugger;
    Entity m_entity;
};