#include "../../includes/tokenizer/token_handler.h"

void TokenHandler::HandleToken(const Token &token) noexcept
{
    bool bIsRelevantToken = false;

    try
    {
        bIsRelevantToken = this->IsRelevantToken(token);
    }
    catch(const std::exception& e)
    {
        return;
    }
    
    return bIsRelevantToken ? token : nullptr;
}

bool TokenHandler::IsRelevantToken(const Token &token) const
{
    const std::string& sTokenType = token.GetType();

    if (sTokenType == k_sUnknownType)
    {
        throw std::runtime_error(k_sTypeError);
    }
    return !(sTokenType == k_sTriviaType|| sTokenType == k_sEOFType);
}