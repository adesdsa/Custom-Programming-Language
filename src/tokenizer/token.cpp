#include "token.h"

Token::Token(size_t ulLine) : m_ulLine(ulLine)
{
    m_sType = k_tokenTypevector[7];
}

bool Token::Append(char cAddedChar) noexcept
{
    std::string sTempToken = m_sValue + cAddedChar;

    if (std::regex_match(sTempToken, k_regexTriviaPattern)) 
        // if it's an UNKNOWN operator
    {
        if (m_sValue.size() == 0) // checking if there was already a token
        {
            m_sValue = sTempToken;
            m_sType = k_tokenTypevector[5];
            return false;
        }
        else { return true; } // the last Token is finished
    }
    else if (std::regex_match(sTempToken, k_regexIdentifierPattern)) 
        // if it's an identifier
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[4];
        return false;
    }
    else if (std::regex_match("" + cAddedChar, k_regexPunctuationPattern)) 
        // if it's a punctuation
    {
        if (m_sValue.size() == 0) // checking if there was already a token
        {
            m_sValue = cAddedChar;
            m_sType = k_tokenTypevector[0];
            return false;
        }
        else { return true; } // the last Token is finished
    }
    else if (std::regex_match(sTempToken, k_regexKeywordPattern)) // if it's a keyword
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[1];
        return false;
    }
    else if (std::regex_match(sTempToken, k_regexValuePattern)) // if it's a keyword
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[2];
        return false;
    }
    else if (std::regex_match(sTempToken, k_regexKeywordSymbolPattern))
        // if it's a symbol operator
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[3];
        return false;
    }
    else if (std::regex_match(sTempToken, k_regexKeywordOperatorPattern))
        // if it's a keyword operator
    {
        if (m_sValue.size() == 0) // checking if there was already a token
        {
            m_sValue = sTempToken;
            m_sType = k_tokenTypevector[3];
            return false;
        }
        else { return true; } // the last Token is finished
    }
    else // if it's an UNKNOWN operator
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[7];
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
