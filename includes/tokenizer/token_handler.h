#pragma once

#include <fstream>

#include "token.h"

#define FUNCTION_THROWER __func__

constexpr const char* k_sOutputFile = "../src/tokenizer/tokens_out.txt";

constexpr const char* k_sTriviaType = "TRIVIA";
constexpr const char* k_sEOFType = "EOF";
constexpr const char* k_sUnknownType = "UNKNOWN";

constexpr const char* k_sFileFailedToOpenError = " - ERROR: Failed to open file: ";
constexpr const char* k_sTypeError = " - ERROR: Type is not valid";

// Responsible for filter tokens from input stream to output stream  
class TokenHandler
{
public:
    TokenHandler();
    
    ~TokenHandler() = default;

    // Preform an action based on th token's relevance.
    // @param token: Current token from stream to handle.
    // @return: None.
    void HandleToken(const Token& token) noexcept;

    // Close stream of tokens.
    // @return: None.
    void CloseStream();

private:
    std::ofstream m_streamTokens;

    // Checks for token relevance.
    // @param token: Current token from stream to check it relevance.
    // @return: True => Token is relevant (not a trivia/unknown type token), False => Otherwise.
    bool IsRelevantToken(const Token& token) const;

    // Adds the relevant tokens to the tokens stream.
    // @param token: Current token from stream to handle.
    // @return: None.
    void ProcessRelevantToken(const Token& token) noexcept;
};