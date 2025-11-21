#include "tokenizer/token_factory.h"

TokenFactory::TokenFactory(
    const std::string& sFilePath,
    const EntityLogger& entityLogger
) :
    m_mappedSRC(sFilePath),
    m_ulLineCounter(1), 
    m_entityLogger(entityLogger) {}

Token TokenFactory::CreateToken()
{
    if (this->IsFinishedReadingSRC())
    {
        return Token(
            TokenTypes::EOF_TOKEN, 
            this->m_ulLineCounter
        );
    }

    Token token(this->m_ulLineCounter);

    while (!token.Append(this->m_mappedSRC.Get()))
    {
        if(this->m_mappedSRC.Get() == '\n')
        {
            this->m_ulLineCounter++;
        }

        try
        {
            (void)this->m_mappedSRC.Seek();
        }
        catch(const std::runtime_error&)
        {
            throw;
        }
    }

    return token;
}

bool TokenFactory::IsFinishedReadingSRC() const noexcept
{
    return this->m_mappedSRC.IsEOF();
}
