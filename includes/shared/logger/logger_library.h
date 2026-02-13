#pragma once

/*******************************************************************************
*** This file contains all the shared resources between the logger component ***
********************************************************************************/

#include <string>
#include <unordered_map>

#define FUNCTION_THROWER __func__
constexpr const char* k_sReset = "\033[0m";

const std::unordered_map<std::string, std::string> ENTITY_COLORS = {
    {"LEXER", "\033[32m"},                  // Green
    {"PARSER", "\033[36m"},                // Cyan
    {"SEMANTIC_ANALYSIS", "\033[35m"},    // Pink (Magneta)
    {"IR GENERATOR", "\033[38;5;208m"},  // Orange
    {"OPTIMIZATION", "\033[38;5;220m"}, // Gold
    {"ASM", "\033[38;5;213m"}, // Pink
    {"ERROR-MANAGER", "\033[31m"},     // Red
    {"GENERAL", "\033[37m"}           // White
};

const std::unordered_map<std::string, std::string> DEBUG_COLORS = {
    { "error",   "\033[31m" },   // Red
    { "warn",    "\033[33m" },  // Yellow
    { "info",    "\033[34m" }  // Blue
};

// Entity that sends the message
struct Entity
{
    std::string sName;
    std::string sColor;

    Entity(const std::string& name, const std::string& color)
        : sName(name), sColor(color) {}
};

// Type of message Info/Warnning/Error
struct DebugLevel
{
    std::string sName;
    std::string sColor;

    DebugLevel(const std::string& name, const std::string& color)
        : sName(name), sColor(color) {}
};

// Message to log with all the information built (entity sender, message type and the content).
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