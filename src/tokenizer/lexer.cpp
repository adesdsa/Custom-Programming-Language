#include "../../includes/tokenizer/lexer.h"

Lexer::Lexer() : m_entityLogger("lexer"), m_tokenFactory(OpenInputFile(k_sInputFile), m_entityLogger), m_tokenHandler(m_entityLogger) {}

Lexer::Lexer(const std::string& sInputFile) : m_entityLogger("lexer"), m_tokenHandler(m_entityLogger), m_tokenFactory(OpenInputFile(sInputFile), m_entityLogger) {}

std::optional<Token> Lexer::Tokenize() 
{
    this->m_entityLogger.Info("Start Creating new token...");
    return this->m_tokenHandler.HandleToken(this->m_tokenFactory.CreateToken());
}

bool Lexer::IsFinishedReadingSRC() const noexcept
{
    return this->m_tokenFactory.IsFinishedReadingSRC();
}

std::ifstream Lexer::OpenInputFile(const std::string& sInputFile)
{
    std::ifstream streamSrcFile(sInputFile);

    if (!streamSrcFile.is_open()) 
    {
        throw std::runtime_error(
            std::string(FUNCTION_THROWER) + 
            k_sFileFailedToOpenError + 
            sInputFile);
    }

    return streamSrcFile;
}