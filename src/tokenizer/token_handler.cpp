#include "../../includes/tokenizer/token_handler.h"

<<<<<<< src/tokenizer/token_handler.cpp
std::optional<Token> TokenHandler::HandleToken(const Token& token) noexcept
=======
TokenHandler::TokenHandler(const EntityLogger& entityLogger) : m_entityLogger(entityLogger) {}

std::optional<Token> TokenHandler::HandleToken(const Token& token) noexcept
>>>>>>> src/tokenizer/token_handler.cpp
{
    this->m_entityLogger.Info("Handle token");
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
    this->m_entityLogger.Info("Check for token relevance");
    const std::string& sTokenType = token.GetType();

    if (sTokenType == k_sUnknownType)
    {
        this->m_entityLogger.Error("Unknown Token");
        throw std::runtime_error(k_sTypeError);
    }
    return sTokenType != k_sTriviaType && sTokenType != k_sEOFType;
}