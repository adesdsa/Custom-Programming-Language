#pragma once

#include "tokenizer/lexer.h"

class Parser
{
public:
    Parser(const std::string& sFilePath);
    ~Parser() = default;

    // Sets m_currentToken to be the m_nextToken
    // and fetch new m_nextToken from lexer
    void Advance();

private:
    EntityLogger m_entityLogger;
    Lexer m_lexer;
    Token m_currentToken;
    Token m_nextToken;

    // Gets new token from lexer
    Token FetchToken();
};
