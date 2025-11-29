#include "tokenizer/token_handler.h"

TokenHandler::TokenHandler(const EntityLogger& entityLogger) : 
    m_entityLogger(entityLogger) {}

std::optional<Token> TokenHandler::HandleToken(const Token& token) noexcept
{
    this->m_entityLogger.Info("Handle token");
    try
    {
        if (this->IsRelevantToken(token))
        {
            return token;
        }
    }
    catch (const std::exception&) { /*skip*/ }

    return std::nullopt;
}

bool TokenHandler::IsRelevantToken(const Token &token) const
{
    this->m_entityLogger.Info("Check for token relevance");
    const std::string& sTokenType = token.GetType();

    if (sTokenType == TokenTypes::UNKNOWN)
    {
        this->m_entityLogger.Error(
            std::string("ERROR: Unknown Token - ") + FUNCTION_THROWER
        );
        throw std::runtime_error("ERROR: Not supported type");
    }
    return sTokenType != TokenTypes::TRIVIA;
}
