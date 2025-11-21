#pragma once

#include "tokenizer/lexer.h"

class Parser
{
public:
    Parser(const std::string& sFilePath);
    ~Parser() = default;

    void Advance();

private:
    EntityLogger m_entityLogger;
    Lexer m_lexer;
    Token m_currentToken;
    Token m_nextToken;

    Token FetchToken();
};