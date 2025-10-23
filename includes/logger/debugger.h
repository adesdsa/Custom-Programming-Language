#pragma once

#include <string>

struct DebugLevel
{
    std::string sName;
    std::string sColor;
};

class Debugger
{
public:
    Debugger() = default;

    ~Debugger() = default;

    //void BuildLogMessage(const DebugLevel& debugLevel, const Entity& entity ,std::string timeStamp);


private:
    // Console& m_pConsole;

    std::string  GetCurrentTime();
};