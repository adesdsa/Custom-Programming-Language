#include "token_handler.h"

void TokenHandler::HandleToken(const Token &token) noexcept
{
    if (!this->IsRelevantToken(token)) { return; }

    this->ProcessRelevantToken(token);
}