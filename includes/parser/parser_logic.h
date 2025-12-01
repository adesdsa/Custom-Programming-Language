#pragma once

#include "ast.h"
#include "commandLine.h"

#include "includes/tokenizer/lexer.h"
#include "includes/errors/error_manager.h"

class ParserLogic
{
public:
    ParserLogic(const std::string& sFilePath);
    ~ParserLogic() = default;

    void MainFlow();

    // Sets m_currentToken to be the m_nextToken
    // and fetch new m_nextToken from lexer
    void Advance();

    // A command line is considered valid if it passed the SyntaxChecker layer.
    // If `fullCommand` is valid -> construct its subtree in the AST.
    // Otherwise -> record the syntax errors in `m_errorManager`.
    // @param fullCommand: The fully command line to analyze and process.
    void HandleFullCommand(const CommandLine& fullCommand);

private:
    // infrastructures
    EntityLogger m_entityLogger;
    ErrorManager& m_errorManager;

    // inner components
    AST m_ast;
    Lexer m_lexer;

    // data
    Token m_currentToken;
    Token m_nextToken;


    // Gets new relevant token from lexer
    Token FetchToken();
};
