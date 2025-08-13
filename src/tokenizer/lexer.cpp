#include "lexer.h"

Lexer::Lexer() : m_tokenHandler(), m_tokenFactory(OpenInputFile()) {};

void Lexer::Tokenize() 
{
    while(!this->m_tokenFactory.IsFinishedReadingSRC())
    {
        this->m_tokenHandler.HandleToken(this->m_tokenFactory.CreateToken());
    }
}

std::ifstream Lexer::OpenInputFile()
{
    std::ifstream streamSrcFile(k_sInputFile);

    if (!streamSrcFile) 
    {
        throw std::runtime_error(
            std::string(FUNCTION_THROWER) + 
            k_sFileFailedToOpenError + 
            k_sInputFile);
    }

    return streamSrcFile;
}