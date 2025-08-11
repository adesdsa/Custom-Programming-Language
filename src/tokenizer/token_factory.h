#pragma once

#include "mapped_src.h"
#include "token.h"

// Will create and build tokens from the stream
class TokenFactory
{
public:
    TokenFactory(std::ifstream& fSrc);

    ~TokenFactory() = default;

    // Will build a new token. 
    // @return: The fully initialized token. 
    Token& CreateToken() noexcept;

    // Checks if file is in EOF.
    // @return: True => , False => .
    bool IsFinishedReadingSRC() const noexcept;


private:
    MappedSRC m_mappedSRC;
    size_t m_ulLineCounter;
};