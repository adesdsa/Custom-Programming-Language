#pragma once

#include "mapped_src.h"
#include "token.h"

// Will create and build tokens from the stream
class TokenFactory
{
public:
    TokenFactory(const std::string sFilePath, const EntityLogger& entityLogger);

    ~TokenFactory() = default;

    // Will build a new token. 
    // @return: The fully initialized token. 
    Token CreateToken() noexcept;

    // Checks if file is in EOF.
    // @return: True => , False => .
    bool IsFinishedReadingSRC() const noexcept;


private:
    MappedSRC m_mappedSRC;
    size_t m_ulLineCounter;
    const EntityLogger& m_entityLogger;
};