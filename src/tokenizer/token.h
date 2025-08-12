#pragma once

#include <string>
#include <vector>

#include "patterns.h"


// TODO: check the maximum scope of the vector
namespace TokenTypes
{
    inline constexpr std::string_view PUNCTUATION = "PUNCTUATION";
    inline constexpr std::string_view KEYWORD     = "KEYWORD";
    inline constexpr std::string_view VALUE       = "VALUE";
    inline constexpr std::string_view OPERATOR    = "OPERATOR";
    inline constexpr std::string_view IDENTIFIER  = "IDENTIFIER";
    inline constexpr std::string_view TRIVIA      = "TRIVIA";
    inline constexpr std::string_view EOF_TOKEN   = "EOF";
    inline constexpr std::string_view UNKNOWN     = "UNKNOWN";
}


// Represent a token in the stream
class Token
{
public:
    Token(size_t ulLine);

     // Tries to append the new character to the Token unless it's a char for
     // the next token than it defines the token type.
     // @param cAddedChar: The character we add to the token right now.
     // @return: True => The token is finished, False => Otherwise.
    bool Append(char cAddedChar) noexcept;

    const std::string& GetValue() const noexcept; 
    const std::string& GetType() const noexcept;
    const std::string& GetLine() const noexcept;

private:
    std::string m_sValue;
    std::string m_sType;
    size_t m_ulLine;

    // Set a single character token.
    // @param sTokenValue: New value of the token.
    // @param sTokenType: Type of the token.
    // @return: True => The token is finished, False => Otherwise.
    bool AssignToken(const std::string& sTokenValue, std::string_view sTokenType);

    // Tries to append the new character to the Token unless it's a char for
    // @param sTokenValue: New value of the token.
    // @param sTokenType: Type of the token.
    // @return: True => The token is finished, False => Otherwise.
    bool AssignIfEmptyOrFinish(const std::string& sTokenValue, std::string_view sTokenType);
};
