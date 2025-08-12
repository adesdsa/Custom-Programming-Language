#pragma once

#include <fstream>

#include "token.h"

class TokenHandler
{
public:
    TokenHandler() = default;
    ~TokenHandler() = default;

    void HandleToken(Token& token) noexcept;


private:
    std::ofstream m_streamTokens;

    bool IsRelevantToken(Token& token) noexcept;
    void SkipIrelevantToken(Token& token) noexcept;
    void ProcessRelevantToken(Token& token) noexcept;
};