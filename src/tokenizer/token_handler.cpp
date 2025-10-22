#include "../../includes/tokenizer/token_handler.h"

std::optional<std::reference_wrapper<const Token>>
TokenHandler::HandleToken(const Token& token) noexcept
{
    try
    {
        if (this->IsRelevantToken(token))
        {
            return token;
        }
    }
    catch (const std::exception& e)
    {
        //
    }

    return std::nullopt;
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