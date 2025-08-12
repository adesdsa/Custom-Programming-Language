#include "token.h"

Token::Token(size_t ulLine) : m_ulLine(ulLine)
{
    this->m_sType = k_tokenTypevector[7];
}

bool Token::Append(char cAddedChar) noexcept
{   
    // For single character tokens
    auto AssignToken = [&](const std::string& val, size_t typeIdx) 
    {
        this->m_sValue = val;
        this->m_sType = k_tokenTypevector[typeIdx];
        return false;
    };

    // For multi-characters tokens
    auto AssignIfEmptyOrFinish = [&](const std::string& val, size_t typeIdx) 
    {
        // Checking if there was already a token
        if (this->m_sValue.empty())
        {
            return AssignToken(val, typeIdx);
        }
        return true;  // Last token finished
    };

    // New token value
    std::string sTempToken = m_sValue + cAddedChar;

    if (std::regex_match(sTempToken, RegexPatterns::k_Trivia)) 
    {
        return AssignIfEmptyOrFinish(sTempToken, 5);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_Identifier)) 
    {
        return AssignToken(sTempToken, 4);
    }

    if (std::regex_match(std::string(1, cAddedChar), RegexPatterns::k_Punctuation)) 
    {
        return AssignIfEmptyOrFinish(std::string(1, cAddedChar), 0);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_Keyword))
    {
        return AssignToken(sTempToken, 1);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_Value))
    {
        return AssignToken(sTempToken, 2);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_KeywordSymbol))
    {
        return AssignToken(sTempToken, 3);
    }

    if (std::regex_match(sTempToken, RegexPatterns::k_KeywordOperator))
    {
        return AssignIfEmptyOrFinish(sTempToken, 3);
    }

    // UNKNOWN token
    return AssignToken(sTempToken, 7);
}

std::string Token::GetValue() const noexcept
{
    return this->m_sValue;
}

std::string Token::GetType() const noexcept
{
    return this->m_sType;
}

std::string Token::GetLine() const noexcept
{
    return std::to_string(this->m_ulLine);
}
