#include "token_factory.h"

TokenFactory::TokenFactory(std::ifstream fSrc) : m_mappedSRC(std::move(fSrc)), m_ulLineCounter(0) {};

Token &TokenFactory::CreateToken() noexcept
{
    Token token(this->m_ulLineCounter);

    while(!token.Append(this->m_mappedSRC.Get()))
    {
        if(this->m_mappedSRC.Get() == '\n')
        {
            this->m_ulLineCounter++;
        }

        this->m_mappedSRC.Seek();
    }

    return token;
}

bool TokenFactory::IsFinishedReadingSRC() const noexcept
{
    return this->m_mappedSRC.IsEOF();
}
