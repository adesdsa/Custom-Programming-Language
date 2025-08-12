#include "token_handler.h"

void TokenHandler::HandleToken(const Token &token) noexcept
{
    if (!this->IsRelevantToken(token)) { return; }

    this->ProcessRelevantToken(token);
}

bool TokenHandler::IsRelevantToken(const Token &token) const noexcept
{
    return !(token.GetType() == k_triviaType|| token.GetType() == k_unknownType);
}

void TokenHandler::ProcessRelevantToken(const Token &token) noexcept
{
    this->m_streamTokens << token.GetLine() << " " << token.GetType() << " " << token.GetValue() << ",";
}
