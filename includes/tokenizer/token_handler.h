#pragma once

#include <fstream>
#include <optional>

#include "token.h"

#define FUNCTION_THROWER __func__

constexpr const char* k_sTriviaType = "TRIVIA";
constexpr const char* k_sEOFType = "EOF";
constexpr const char* k_sUnknownType = "UNKNOWN";

constexpr const char* k_sFileFailedToOpenError = " - ERROR: Failed to open file: ";
constexpr const char* k_sTypeError = " - ERROR: Type is not valid";

// Responsible for filter tokens from input stream to output stream  
class TokenHandler
{
public:
    TokenHandler(const EntityLogger& entityLogger);
    
    ~TokenHandler() = default;

    // Preform an action based on th token's relevance.
    // @param token: Current token from stream to handle.
    // @return: Relevant token => Reference to the token. Otherwise => nullopt. 
    std::optional<std::reference_wrapper<const Token>> HandleToken(const Token& token) noexcept;


private:
    const EntityLogger& m_entityLogger;

    // Checks for token relevance.
    // @param token: Current token from stream to check it relevance.
    // @return: True => Token is relevant (not a trivia/unknown type token), False => Otherwise.
    bool IsRelevantToken(const Token& token) const;
};