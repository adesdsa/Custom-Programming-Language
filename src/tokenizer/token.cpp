#include "token.h"

Token::Token(size_t ulLine) noexcept: m_ulLine(ulLine)
{
    m_sType = k_tokenTypevector[7];
}

bool Token::Append(char cAddedChar) noexcept
{
    std::string sTempToken = m_sValue + cAddedChar;

    std::regex identifierPattern(R"(^[A-Za-z_][A-Za-z0-9_]*$)", 
        std::regex::optimize);
    std::regex punctuationPattern(R"(::|->|##|;|,|\.|:|\?|#|\{|\}|\(|\)|\[|\])" 
        ,std::regex::optimize);
    std::regex keywordPattern(R"(\b(?:alignas|alignof|and|and_eq|asm|auto|
    bitand|bitor|bool|break|case|catch|char|char8_t|char16_t|char32_t|class|
    compl|concept|const|consteval|constexpr|constinit|const_cast|continue|
    co_await|co_return|co_yield|decltype|default|delete|do|double|dynamic_cast|
    else|enum|explicit|export|extern|false|float|for|friend|goto|if|inline|int|
    long|mutable|namespace|new|noexcept|not|not_eq|nullptr|operator|or|or_eq|
    private|protected|public|reflexpr|register|reinterpret_cast|requires|
    return|short|signed|sizeof|static|static_assert|static_cast|struct|switch|
    synchronized|template|this|thread_local|throw|true|try|typedef|typeid|
    typename|union|unsigned|using|virtual|void|volatile|wchar_t|while|xor|
    xor_eq)\b)", std::regex::optimize);
    std::regex valuePattern(R"(\b(?:true|false|nullptr|NULL)\b
        |(?:0b[01]+|0x[0-9A-Fa-f]+|0[0-7]+|[0-9]+)(?:[uU]?(?:l{1,2}|L{1,2})?)?
        |\d+\.\d*(?:[eE][+-]?\d+)?[fFlL]?
        |\d+(?:[eE][+-]?\d+)[fFlL]?
        |'(?:\\.|[^\\'])'
        |"(?:\\.|[^\\"])*")",
        std::regex::optimize);
    std::regex keywordOperatorPattern(R"(\b(?:sizeof|alignof|typeid|decltype|
    new|delete|co_await)\b")", std::regex::optimize);
    std::regex keywordSymbolPattern(R"(R"(|(?:\+\+|--|==|!=|<=|>=|&&|\|\||
    \+=|-=|\*=|/=|%=|\^=|&=|\|=|<<=|>>=|<<|>>|->\*|->|\.|\.\*|::|[+\-*/%^&|
    ~!<>=?:]))", std::regex::optimize);
    std::regex triviaPattern(R"(? : \s + |//[^\n]*|/\*[\s\S]*?\*/)",
        std::regex::optimize);

    if (std::regex_match(sTempToken, triviaPattern)) 
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
    else if (std::regex_match(sTempToken, identifierPattern)) 
        // if it's an identifier
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[4];
        return false;
    }
    else if (std::regex_match("" + cAddedChar, punctuationPattern)) 
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
    else if (std::regex_match(sTempToken, keywordPattern)) // if it's a keyword
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[1];
        return false;
    }
    else if (std::regex_match(sTempToken, valuePattern)) // if it's a keyword
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[2];
        return false;
    }
    else if (std::regex_match(sTempToken, keywordSymbolPattern))
        // if it's a symbol operator
    {
        m_sValue = sTempToken;
        m_sType = k_tokenTypevector[3];
        return false;
    }
    else if (std::regex_match(sTempToken, keywordOperatorPattern))
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
