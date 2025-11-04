#include "../../includes/tokenizer/lexer.h"

Lexer::Lexer() : m_tokenFactory(OpenInputFile(k_sInputFile)), m_tokenHandler() {};

Lexer::Lexer(const std::string& sInputFile) : m_tokenHandler(), m_tokenFactory(OpenInputFile(sInputFile)) {};

std::optional<std::reference_wrapper<const Token>> Lexer::Tokenize() 
{
    return this->m_tokenHandler.HandleToken(this->m_tokenFactory.CreateToken());
}

bool Lexer::IsFinishedReadingSRC() const noexcept
{
    return this->m_tokenFactory.IsFinishedReadingSRC();
}

bool operator==(const Token& other) const 
{
    return _type == other._type && _value == other._value;
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