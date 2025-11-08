#include "../../includes/tokenizer/token_factory.h"

TokenFactory::TokenFactory(boost::iostreams::mapped_file_source fSrc, const EntityLogger& entityLogger) : m_mappedSRC(std::move(fSrc)), m_ulLineCounter(0), m_entityLogger(entityLogger) {}

Token TokenFactory::CreateToken() noexcept
{
    if (this->IsFinishedReadingSRC())
    {
        return Token(TokenTypes::EOF_TOKEN, this->m_ulLineCounter, this->m_entityLogger);
    }

    Token token(this->m_ulLineCounter, this->m_entityLogger);

    while (!token.Append(this->m_mappedSRC.Get()))
    {
        if(this->m_mappedSRC.Get() == '\n')
        {
            this->m_ulLineCounter++;
        }

        (void)this->m_mappedSRC.Seek();
    }

    return token;
}

bool TokenFactory::IsFinishedReadingSRC() const noexcept
{
    return this->m_mappedSRC.IsEOF();
}
