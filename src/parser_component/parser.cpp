#include "parser_component/parser.h"

Parser::Parser(const std::string& sFilePath) :
    m_entityLogger("parser"),
    m_lexer(sFilePath),
    m_currentToken(this->FetchToken()),
    m_nextToken(this->FetchToken()) {}

Token Parser::FetchToken()
{
    auto token = this->m_lexer.Tokenize();

    if (!token)
    {
        throw std::runtime_error("Lexer returned no token"); 
    }
    return std::move(*token);
}
