#include "token.h"

Token::Token(size_t ulLine) : m_ulLine(ulLine), m_sType(TokenTypes::UNKNOWN) {};

bool Token::Append(char cAddedChar) noexcept
{   
    // For single character tokens
    auto AssignToken = [&](const std::string& val, std::string_view type) 
    {
        this->m_sValue = val;
        this->m_sType = type;
        return false;
    };

    // For multi-characters tokens
    auto AssignIfEmptyOrFinish = [&](const std::string& val, std::string_view type) 
    {
        // Checking if there was already a token
        if (this->m_sValue.empty())
        {
            return AssignToken(val, type);
        }
        return true;  // Last token finished
    };

    // New token value
    std::string sTempToken = m_sValue + cAddedChar;

    if (std::regex_match(sTempToken, RegexPatterns::k_Trivia)) 
    {
        return AssignIfEmptyOrFinish(sTempToken, TokenTypes::TRIVIA);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_Identifier)) 
    {
        return AssignToken(sTempToken, TokenTypes::IDENTIFIER);
    }

    if (std::regex_match(std::string(1, cAddedChar), RegexPatterns::k_Punctuation)) 
    {
        return AssignIfEmptyOrFinish(std::string(1, cAddedChar), TokenTypes::PUNCTUATION);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_Keyword))
    {
        return AssignToken(sTempToken, TokenTypes::KEYWORD);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_Value))
    {
        return AssignToken(sTempToken, TokenTypes::VALUE);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_KeywordSymbol))
    {
        return AssignToken(sTempToken, TokenTypes::OPERATOR);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_KeywordOperator))
    {
        return AssignIfEmptyOrFinish(sTempToken, TokenTypes::OPERATOR);
    }

    // UNKNOWN token
    return AssignToken(sTempToken, TokenTypes::UNKNOWN);
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

bool Token::AssignToken(const std::string &sTokenValue, std::string_view sTokenType)
{
    this->m_sValue = sTokenValue;
    this->m_sType = sTokenType;
    return false;
}
