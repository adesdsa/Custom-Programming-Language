#include "token_factory.h"

void TokenFactory::CreateToken()
{
    Token token(this->m_ulLineCounter);

    while(!token->Append(this->m_mappedSRC.Get()))
    {
        if(this->m_mappedSRC.Get() == '\n')
        {
            this->m_ulLineCounter++;
        }

        this->m_mappedSRC.Seek();
    }

    return token;
}