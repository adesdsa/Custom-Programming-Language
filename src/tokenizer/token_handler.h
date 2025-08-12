#pragma once

#include <fstream>

#include "token.h"

constexpr const char* k_triviaType = "TRIVIA";
constexpr const char* k_unknownType = "UNKNOWN";

// Responsible for filter tokens from input stream to output stream  
class TokenHandler
{
public:
    TokenHandler() = default;
    ~TokenHandler() = default;

    // Preform an action based on th token's relevance.
    // @param token: Current token from stream to handle.
    // @return: None.
    void HandleToken(const Token& token) noexcept;


private:
    std::ofstream m_streamTokens;

    // Checks for token relevance.
    // @param token: Current token from stream to check it relevance.
    // @return: True => Token is relevant (not a trivia/unknown type token), False => Otherwise.
    bool IsRelevantToken(const Token& token) const noexcept;

    // Adds the relevant tokens to the tokens stream.
    // @param token: Current token from stream to handle.
    // @return: None.
    void ProcessRelevantToken(const Token& token) noexcept;
};