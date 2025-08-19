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
    bool bIsRelevantToken = false;

    // Incase of unkown token error thrown
    try
    {
        bIsRelevantToken = this->IsRelevantToken(token);
    }
    catch(const std::exception& e)
    {
        return;
    }
    
    if(bIsRelevantToken)
    {
        this->ProcessRelevantToken(token);
    }
}

bool TokenHandler::IsRelevantToken(const Token &token) const
{
    const std::string& sTokenType = token.GetType();

    if (sTokenType == k_sUnknownType)
    {
        throw std::exception(k_sTypeError);
    }
    return !(sTokenType == k_sTriviaType|| sTokenType == k_sEOFType);
}

void TokenHandler::ProcessRelevantToken(const Token &token) noexcept
{
    this->m_streamTokens << token.GetLine() << " " << token.GetType() << " " << token.GetValue() << ",";
}
