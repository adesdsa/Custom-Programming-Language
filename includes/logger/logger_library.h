#pragma once

/*******************************************************************************
*** This file contains all the shared resources between the logger component ***
********************************************************************************/

#include <string>
#include <unordered_map>


constexpr const char* k_sReset = "\033[0m";

const std::unordered_map<std::string, std::string> ENTITY_COLORS = {
    { "lexer",   "\033[32m" },   // Green
    { "parser",    "\033[36m" }  // Cyan
};

const std::unordered_map<std::string, std::string> DEBUG_COLORS = {
    { "error",   "\033[31m" },   // Red
    { "warn",    "\033[33m" },  // Yellow
    { "info",    "\033[34m" }  // Blue
};

struct Entity
{
    std::string sName;
    std::string sColor;

    Entity(const std::string& name, const std::string& color)
        : sName(name), sColor(color) {}
};

struct DebugLevel
{
    std::string sName;
    std::string sColor;

    DebugLevel(const std::string& name, const std::string& color)
        : sName(name), sColor(color) {}
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