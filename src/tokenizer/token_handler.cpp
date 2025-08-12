#include "token_handler.h"

TokenHandler::TokenHandler()
{
    this->m_streamTokens.open(k_sOutputFile);

    if (!this->m_streamTokens.is_open())
    {
        throw std::runtime_error(
            std::string(FUNCTION_THROWER) + 
            k_sFileFailedToOpenError + 
            k_sOutputFile
        );
    }
}

void TokenHandler::HandleToken(const Token &token) noexcept
{
    if (this->IsRelevantToken(token))
    {
        this->ProcessRelevantToken(token);
    }
}

bool TokenHandler::IsRelevantToken(const Token &token) const noexcept
{
    return !(token.GetType() == k_sTriviaType|| token.GetType() == k_sUnknownType);
}

void TokenHandler::ProcessRelevantToken(const Token &token) noexcept
{
    this->m_streamTokens << token.GetLine() << " " << token.GetType() << " " << token.GetValue() << ",";
}
