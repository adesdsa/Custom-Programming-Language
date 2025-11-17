#include "parser_component/parser.h"
#include "parser.h"

/*
Parser::Parser(const std::string& sFilePath) :
    m_entityLogger("parser"),
    m_lexer(sFilePath)
{
    this->m_currentToken = this->m_lexer.Tokenize();
    this->m_nextToken = this->m_lexer.Tokenize();
} */

/*
const Token& Parser::GetNewToken() const
{
    try
    {        
        while(!this->m_lexer.IsFinishedReadingSRC()) 
        {
            auto token = this->m_lexer.Tokenize();

            if(token.has_value())
            {
                return token.value();
            }
        }
    }
    catch(std::runtime_error&) throw;
    catch(std::exception&) throw;
}
*/