#pragma once

#include "mapped_src.h"

// Will create and build tokens from the stream
class TokenFactory
{
 public:
    TokenFactory(std::ifstream& fSrc);

    ~TokenFactory() = default;

    // Will build a new token. 
    // @return: The fully initialized token. 
    void CreateToken();

    // Checks if file is in EOF.
    // @return: True => , False => .
    bool IsFinishedReadingSRC();

 private:
    MappedSRC m_mappedSRC;
    size_t m_ulLineCounter;
};