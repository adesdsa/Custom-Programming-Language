#include "token.h"

Token::Token(size_t ulLine) : m_ulLine(ulLine)
{
    this->m_sType = k_tokenTypevector[7];
}

bool Token::Append(char cAddedChar) noexcept
{
    std::string sTempToken = m_sValue + cAddedChar;

    if (std::regex_match(sTempToken, RegexPatterns::k_Trivia)) 
    {
        if (this->m_sValue.empty()) // checking if there was already a token
        {
            this->m_sValue = sTempToken;
            this->m_sType = k_tokenTypevector[5];
            return false;
        }
        else { return true; } // the last Token is finished
    }

    else if (std::regex_match(sTempToken, RegexPatterns::k_Identifier)) 
    {
        this->m_sValue = sTempToken;
        this->m_sType = k_tokenTypevector[4];
        return false;
    }

    else if (std::regex_match("" + cAddedChar, RegexPatterns::k_Punctuation)) 
    {
        if (this->m_sValue.empty()) // checking if there was already a token
        {
            this->m_sValue = cAddedChar;
            this->m_sType = k_tokenTypevector[0];
            return false;
        }
        else { return true; } // the last Token is finished
    }

    else if (std::regex_match(sTempToken, RegexPatterns::k_Keyword))
    {
        this->m_sValue = sTempToken;
        this->m_sType = k_tokenTypevector[1];
        return false;
    }

    else if (std::regex_match(sTempToken, RegexPatterns::k_Value))
    {
        this->m_sValue = sTempToken;
        this->m_sType = k_tokenTypevector[2];
        return false;
    }

    else if (std::regex_match(sTempToken, RegexPatterns::k_KeywordSymbol))
    {
        this->m_sValue = sTempToken;
        this->m_sType = k_tokenTypevector[3];
        return false;
    }

    else if (std::regex_match(sTempToken, RegexPatterns::k_KeywordOperator))
    {
        if (this->m_sValue.empty()) // checking if there was already a token
        {
            this->m_sValue = sTempToken;
            this->m_sType = k_tokenTypevector[3];
            return false;
        }
        else { return true; } // the last Token is finished
    }

    else // UNKNOWN token
    {
        this->m_sValue = sTempToken;
        this->m_sType = k_tokenTypevector[7];
        return false;
    }
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
