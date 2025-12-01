#pragma once

#include "tokenizer/lexer.h"
#include "ast.h"
#include "commandLine.h"

class Parser
{
public:
    Parser(const std::string& sFilePath);
    ~Parser() = default;

    void MainFlow();

    // Sets m_currentToken to be the m_nextToken
    // and fetch new m_nextToken from lexer
    void Advance();

private:
    EntityLogger m_entityLogger;
    AST m_ast;
    Lexer m_lexer;
    Token m_currentToken;
    Token m_nextToken;

    // Gets new token from lexer
    Token FetchToken();
};
