#include "includes/frontend/tokenizer/token_factory.h"

TokenFactory::TokenFactory(
    const std::filesystem::path& filePath,
    const EntityLogger& entityLogger
) :
    m_mappedSRC(filePath),
    m_ulLineCounter(1), 
    m_ulColumnCounter(1),
    m_entityLogger(entityLogger) 
{}

Token TokenFactory::CreateToken()
{
    if (this->IsFinishedReadingSRC())
    {
        return Token(
            TokenTypes::EOF_TOKEN, 
            this->m_ulLineCounter,
            this->m_ulColumnCounter
        );
    }

    Token token(this->m_ulLineCounter, this->m_ulColumnCounter);

    while (!token.Append(this->m_mappedSRC.Get()))
    {
        if (this->m_mappedSRC.Get() == '\n')
        {
            this->m_ulLineCounter++;
            this->m_ulColumnCounter = 1;  // reset column at new line
        }
        else
        {
            this->m_ulColumnCounter++;
        }

        (void)this->m_mappedSRC.Seek();
    }

    return token;
}

bool TokenFactory::IsFinishedReadingSRC() const noexcept
{
    return this->m_mappedSRC.IsEOF();
} 
