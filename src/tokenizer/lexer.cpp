#include "../../includes/tokenizer/lexer.h"

Lexer::Lexer() : m_entityLogger("lexer"), m_tokenFactory(k_sInputFile, m_entityLogger), m_tokenHandler(m_entityLogger) {}

Lexer::Lexer(const std::string& sInputFile) : m_entityLogger("lexer"), m_tokenHandler(m_entityLogger), m_tokenFactory(sInputFile, m_entityLogger) {}

std::optional<Token> Lexer::Tokenize() 
{
    this->m_entityLogger.Info("Start Creating new token...");
    return this->m_tokenHandler.HandleToken(this->m_tokenFactory.CreateToken());
}

bool Lexer::IsFinishedReadingSRC() const noexcept
{
    return this->m_tokenFactory.IsFinishedReadingSRC();
}

const EntityLogger& Lexer::GetEntityLogger()
{
    return this->m_entityLogger;
}
