#include "includes/tokenizer/lexer.h"

Lexer::Lexer(const std::string& sInputFile) : 
    m_entityLogger("LEXER"), 
    m_tokenFactory(sInputFile, m_entityLogger),
    m_tokenHandler(m_entityLogger) 
{
    this->m_entityLogger.Info("Finished initializing lexer component successfully");
}

std::optional<Token> Lexer::Tokenize()
{
    this->m_entityLogger.Info("Start Creating new token...");

    while (!this->IsFinishedReadingSRC()) 
    {
        auto token = this->m_tokenHandler.HandleToken(
            this->m_tokenFactory.CreateToken()
        );
        
        if (token.has_value())
        {
            return token;
        }
    }

    // Gets the EOF token
    return std::optional<Token>{ 
        this->m_tokenFactory.CreateToken() 
    };
}

bool Lexer::IsFinishedReadingSRC() const noexcept
{
    return this->m_tokenFactory.IsFinishedReadingSRC();
}

const EntityLogger& Lexer::GetEntityLogger() const noexcept
{
    return this->m_entityLogger;
} 
