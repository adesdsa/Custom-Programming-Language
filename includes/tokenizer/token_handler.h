#pragma once

#include <fstream>
#include <optional>

#include "token.h"

// Responsible for filter tokens from input stream to output stream  
class TokenHandler
{
public:
    TokenHandler(const EntityLogger& entityLogger);
    
    ~TokenHandler() = default;

    // Preform an action based on th token's relevance.
    // @param token: Current token from stream to handle.
    // @return: token -> If relevant for code logic. nullopt -> Otherwise. 
    std::optional<Token> HandleToken(const Token& token) noexcept;

private:
    const EntityLogger& m_entityLogger;

    // Checks for token relevance.
    // @param token: Current token from stream to check it relevance.
    // @return: true -> Token is relevant for code logic, False -> Otherwise.
    bool IsRelevantToken(const Token& token) const;
};