#pragma once

#include "mapped_src.h"

class TokenFactory
{
 public:
    TokenFactory(std::ifstream& fSrc);

    ~TokenFactory() = default;

    void CreateToken();

    bool IsFinishedReadingSRC();


 private:
    MappedSRC m_mappedSRC;
    size_t m_ulLineCounter;
}