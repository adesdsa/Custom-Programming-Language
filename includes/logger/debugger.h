#pragma once

#include "console.h"

// Responsible to build the log messages
class Debugger
{
public:
    Debugger() = default;

    ~Debugger() = default;

    //void BuildLogMessage(const DebugLevel& debugLevel, const Entity& entity ,std::string timeStamp);


private:
    Console& m_pConsole;

    std::string  GetCurrentTime();
};