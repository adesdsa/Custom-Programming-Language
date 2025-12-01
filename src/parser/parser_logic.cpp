#include "includes/parser/parser_logic.h"

Parser::Parser(const std::string& sFilePath) :
    m_entityLogger("PARSER"),
    m_ast(m_entityLogger),
    m_lexer(sFilePath),
    m_currentToken(this->FetchToken()),
    m_nextToken(this->FetchToken()) 
{
    this->m_entityLogger.Info("Finished initializing parser component successfully");
}

void Parser::MainFlow()
{
    this->m_entityLogger.Info("Start parsing file");
    while(!this->m_lexer.IsFinishedReadingSRC())
    {
        CommandLine cl(this->m_entityLogger);
        while(!cl.IsFinished())
        {
            cl.AddToken(this->m_currentToken, this->m_nextToken);
            this->Advance();
        }   
    /*
    auto error = cl.IsLegal();
    if(error.has_value())
    {
        m_errormanger.StoreeRROR(error.value();
    }
    else
    {
    */
        this->m_ast.StoreFullCommand(cl);
    }
    this->m_entityLogger.Info("Finished building AST");
    this->m_ast.PrintAST();
}

Token Parser::FetchToken()
{
    auto token = this->m_lexer.Tokenize();

    if (!token)
    {
        this->m_entityLogger.Error("Failed to fetch new token from lexer");
        throw std::runtime_error("Lexer returned no token");
    }
    return std::move(*token);
}

void Parser::Advance()
{
    Token nextToken = this->FetchToken();
    this->m_currentToken = std::move(this->m_nextToken);
    this->m_nextToken = std::move(nextToken);
} 
