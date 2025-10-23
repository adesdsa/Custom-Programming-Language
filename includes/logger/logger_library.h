#pragma once

/*******************************************************************************
*** This file contains all the shared resources between the logger component ***
********************************************************************************/

#include <string>

struct Entity
{
    std::string sName;
    std::string sColor;
};

struct DebugLevel
{
    std::string sName;
    std::string sColor;
};

struct LogMessage
{
    Entity entity;
    DebugLevel debugLevel;
    std::string sTimeStamp;
    std::string sContent;
};