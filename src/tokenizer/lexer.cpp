#include "../../includes/tokenizer/lexer.h"

Lexer::Lexer(const std::string& sInputFile) : 
    m_entityLogger("lexer"), 
    m_tokenFactory(sInputFile, m_entityLogger),
    m_tokenHandler(m_entityLogger) {}

std::optional<Token> Lexer::Tokenize()
{
    this->m_entityLogger.Info("Start Creating new token...");
    try
    {
        return this->m_tokenHandler.HandleToken(this->m_tokenFactory.CreateToken());
    }
    catch(const std::runtime_error&)
    {
        throw;
    }
}

bool Lexer::IsFinishedReadingSRC() const noexcept
{
    return this->m_tokenFactory.IsFinishedReadingSRC();
}

const EntityLogger& Lexer::GetEntityLogger() const noexcept
{
    return this->m_entityLogger;
} 
