#include "token.h"

Token::Token(size_t ulLine) : m_sValue("") , m_ulLine(ulLine), m_sType(TokenTypes::UNKNOWN) {};

Token::Token(std::string_view sTokenType, size_t ulLine) : 
    m_sValue(""), 
    m_ulLine(ulLine), 
    m_sType(sTokenType) {};

bool Token::Append(char cAddedChar) noexcept
{   
    // New token value
    const std::string k_sTempToken = m_sValue + cAddedChar;

    // Some of the conditions have checks like:
    //   std::regex_match(std::string(1, cAddedChar), RegexPatterns::k_Trivia()) 
    //       && this->m_sType != TokenTypes::TRIVIA
    //
    // The purpose of this is to handle cases where `cAddedChar` could be the *first* 
    // character of a new token type. It should only be treated as the start of that 
    // token if the previous token type (`m_sType`) was different. 
    //
    // In other words, a character can begin a new token of type X only if we are not 
    // already inside a token of type X.


    if (
        std::regex_match(k_sTempToken, RegexPatterns::k_Trivia()) ||
        (
            std::regex_match(std::string(1, cAddedChar), RegexPatterns::k_Trivia()) && 
            this->m_sType != TokenTypes::TRIVIA
        )
    )
    {
        return this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::TRIVIA);
    }
    else if (this->m_sType == TokenTypes::TRIVIA)
    {
        return TokenStatus::k_bFinished;
    }

    if (
        std::regex_match(k_sTempToken, RegexPatterns::k_Punctuation()) ||
        (
            std::regex_match(std::string(1, cAddedChar), RegexPatterns::k_Punctuation()) &&
            this->m_sType != TokenTypes::PUNCTUATION
        )
    )
    {
        return this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::PUNCTUATION);
    }
    else if (this->m_sType == TokenTypes::PUNCTUATION)
    {
        return TokenStatus::k_bFinished;
    }

    if (std::regex_match(k_sTempToken, RegexPatterns::k_Keyword()))
    {
        return this->AssignToken(k_sTempToken, TokenTypes::KEYWORD);
    }

    if (
        std::regex_match(k_sTempToken, RegexPatterns::k_Operator()) ||
        (
            std::regex_match(std::string(1, cAddedChar), RegexPatterns::k_Operator()) &&
            this->m_sType != TokenTypes::OPERATOR
        )
    )
    {
        return this->AssignToken(k_sTempToken, TokenTypes::OPERATOR);
    }
    else if (this->m_sType == TokenTypes::OPERATOR) 
    {
        return TokenStatus::k_bFinished;
    }

    if (std::regex_match(k_sTempToken, RegexPatterns::k_Value()))
    {
        return this->AssignToken(k_sTempToken, TokenTypes::VALUE);
    }

    if (std::regex_match(k_sTempToken, RegexPatterns::k_Identifier()))
    {
        return this->AssignToken(k_sTempToken, TokenTypes::IDENTIFIER);
    }

    if (cAddedChar == '\0')
    {
        return this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::EOF_TOKEN);
    }

    return this->AssignToken(k_sTempToken, TokenTypes::UNKNOWN);
}

const std::string& Token::GetValue() const noexcept
{
    return this->m_sValue;
}

const std::string& Token::GetType() const noexcept
{
    return this->m_sType;
}

const std::string& Token::GetLine() const noexcept
{
    return std::to_string(this->m_ulLine);
}

bool Token::AssignToken(const std::string &sTokenValue, std::string_view sTokenType) noexcept
{
    this->m_sValue = sTokenValue;
    this->m_sType = sTokenType;
    return TokenStatus::k_bUnfinished;
}

bool Token::AssignIfEmptyOrFinish(const std::string &sTokenValue, std::string_view sTokenType) noexcept
{
    // Checking if there was already a token
    if (this->m_sValue.empty() || this->m_sType == TokenTypes::UNKNOWN || this->m_sType == sTokenType)
    {
        return AssignToken(sTokenValue, sTokenType);
    }
    return TokenStatus::k_bFinished;
}
