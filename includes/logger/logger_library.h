#pragma once

/*******************************************************************************
*** This file contains all the shared resources between the logger component ***
********************************************************************************/

#include <string>
#include <unordered_map>

const std::unordered_map<LogLevel, std::string> DEBUG_COLORS = {
    { "error",   "\033[31m" },   // Red
    { "warn",    "\033[33m" },  // Yellow
    { "info",    "\033[34m" }  // Blue
};

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

    LogMessage(const Entity& entity, const DebugLevel& debugLevel, 
        const std::string& sTimeStamp, const std::string sContent)
        : entity(entity), debugLevel(debugLevel), sTimeStamp(sTimeStamp), sContent(sContent) {}
};