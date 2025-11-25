#pragma once

#include <vector>

#include "patterns.h"
#include "logger/entity_logger.h"

// All supported token types
namespace TokenTypes
{
    inline constexpr std::string_view PUNCTUATION       = "PUNCTUATION";
    inline constexpr std::string_view KEYWORD           = "KEYWORD";
    inline constexpr std::string_view VALUE             = "VALUE";
    inline constexpr std::string_view OPERATOR          = "OPERATOR";
    inline constexpr std::string_view IDENTIFIER        = "IDENTIFIER";
    inline constexpr std::string_view TRIVIA            = "TRIVIA";
    inline constexpr std::string_view UNFINISHED_TRIVIA = "UNFINISHED_TRIVIA";
    inline constexpr std::string_view EOF_TOKEN         = "EOF";
    inline constexpr std::string_view UNKNOWN           = "UNKNOWN";

    // This types are used in the parser.
    inline constexpr std::string_view HAS_PARAM         = "HAS_PARAM";
}

// Token status holds which part of the building process the token at
namespace TokenStatus
{
    inline constexpr const bool k_bFinished = true;
    inline constexpr const bool k_bUnfinished = false;
}

// Represent a token in the stream
class Token
{
public:
    Token(size_t ulLine, size_t ulColumn);
    Token(std::string_view sTokenType, size_t ulLine, size_t ulColumn);
    Token(const std::string& sTokenValue, std::string_view sTokenType, size_t ulLine, size_t ulColumn);

    ~Token() = default;

    // Tries to append a new character to the current Token.
    // This method updates the token value and may determine the token type for the next token.
    // @param cAddedChar: The character we add to the token right now.
    // @return: true -> The token is finished, false -> Otherwise.
    bool Append(char cAddedChar) noexcept;

    // Getters
    const std::string& GetValue() const noexcept; 
    const std::string& GetType() const noexcept;
    size_t GetLine() const noexcept;
    size_t GetColumn() const noexcept;

    // Operators
    bool operator==(const Token& other) const;

private:
    std::string m_sValue;
    std::string m_sType;
    size_t m_ulLine;
    size_t m_ulColumn;

    // Set a single character token.
    // @param sTokenValue: New value of the token.
    // @param sTokenType: Type of the token.
    // @return: true -> The token is finished, false -> Otherwise.
    bool AssignToken(const std::string& sTokenValue, std::string_view sTokenType) noexcept;

    // Tries to append the new character to the Token unless it's a char for.
    // @param sTokenValue: New value of the token.
    // @param sTokenType: Type of the token.
    // @return: true -> The token is finished, false -> Otherwise.
    bool AssignIfEmptyOrFinish(const std::string& sTokenValue, std::string_view sTokenType) noexcept;

    // Will check if the current Value Token is a valid value, or an unfinished string literal or char literal.
    // @param cFirstLetter: The first letter of the value token.
    // @param cLastLetter: THe last letter of the vlue token.
    // @return: VALUE Token Type -> Valid value, UNKOWN Token Type -> Otherwie.
    std::string_view CheckIfValueIsKnown() const noexcept;

    // Will check if the letter added right now is a guarnteed stop.
    // @param k_cAddedChar: The letter being added right now.
    // @param k_regexCheck: The regex check we need to do.
    // @param k_sCurrentType: The current type.
    // @return: true -> The added char is a guarnteed stop, false -> Otherwise.
    bool CheckSoloLetters(const char k_cAddedChar, const std::regex& k_regexCheck, const std::string_view& k_sCurrentType) const noexcept;

    // Following Method share same logic: Check if given token matches pattern X (e.g., keyword, trivia etc...):
    bool IsCommentToken(const std::string& sToken) const;
    bool IsWhiteSpaceToken(const std::string& sToken, char newChar) const;
    bool IsPunctuationToken(const std::string& sToken, char newChar) const;
    bool IsKeywordToken(const std::string& sToken) const;
    bool IsOperatorToken(const std::string& sToken, char newChar) const;
    bool IsValueToken(const std::string& sToken) const;
    bool IsIdentifierToken(const std::string& sToken) const;
};
