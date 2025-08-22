#include "tokenizer/lexer.h"

Lexer::Lexer() : m_tokenHandler(), m_tokenFactory(OpenInputFile(k_sInputFile)) {};

Lexer::Lexer(const std::string& sInputFile) : m_tokenHandler(), m_tokenFactory(OpenInputFile(sInputFile)) {};

void Lexer::Tokenize() 
{
    while(!this->m_tokenFactory.IsFinishedReadingSRC())
    {
        this->m_tokenHandler.HandleToken(this->m_tokenFactory.CreateToken());
    }
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