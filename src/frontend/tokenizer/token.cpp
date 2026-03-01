#include "includes/frontend/tokenizer/token.h"

Token::Token(size_t ulLine, size_t ulColumn) :
    m_sValue(""), 
    m_sType(TokenTypes::UNKNOWN), 
    m_ulLine(ulLine),
    m_ulColumn(ulColumn) 
{}

Token::Token(std::string_view sTokenType, size_t ulLine, size_t ulColumn) :
    m_sValue(""), 
    m_sType(sTokenType),
    m_ulLine(ulLine),
    m_ulColumn(ulColumn) {}

Token::Token(
        const std::string& sTokenValue,
        std::string_view sTokenType,
        size_t ulLine,
        size_t ulColumn
    ) :
        m_sValue(sTokenValue),
        m_sType(sTokenType),
        m_ulLine(ulLine),
        m_ulColumn(ulColumn) {}

bool Token::Append(char cAddedChar) noexcept
{   
    const std::string k_sTempToken = m_sValue + cAddedChar;

    // if we are already building a string literal, keep accumulating.
    if (this->m_sType == TokenTypes::STRING_LITERAL)
    {
        if (this->m_sValue.size() > 1 && this->m_sValue.back() == '"')
        {
            return TokenStatus::k_bFinished;
        }
        return AssignToken(k_sTempToken, TokenTypes::STRING_LITERAL);
    }

    if (cAddedChar == '"')
    {
        if (!m_sValue.empty())
        {
            return TokenStatus::k_bFinished;
        }
        return AssignToken(k_sTempToken, TokenTypes::STRING_LITERAL);
    }

    // check #1: Comments
    if (this->IsCommentToken(k_sTempToken))
        return this->AssignToken(k_sTempToken, TokenTypes::UNFINISHED_TRIVIA);

    else if (this->m_sType == TokenTypes::UNFINISHED_TRIVIA)
    {
        this->m_sType = TokenTypes::TRIVIA;
        return TokenStatus::k_bFinished;
    }

    // check #2: Whitespace
    if (this->IsWhiteSpaceToken(k_sTempToken, cAddedChar))
        return this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::TRIVIA);

    else if (this->m_sType == TokenTypes::TRIVIA)
        return TokenStatus::k_bFinished;

    // check #3: Punctuation and String Literals
    if (this->IsPunctuationToken(k_sTempToken, cAddedChar))
        // checks if the punctuation is -> then we have to make it not an 
        // operator but a punctuation.
        return (cAddedChar == '>' && this->m_sValue == "-") 
        ? this->AssignToken(k_sTempToken, TokenTypes::PUNCTUATION) 
        : this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::PUNCTUATION);

    else if (this->m_sType == TokenTypes::PUNCTUATION)
        return TokenStatus::k_bFinished;

    // Check #4: Keyword
    if (this->IsKeywordToken(k_sTempToken))
        return this->AssignToken(k_sTempToken, TokenTypes::KEYWORD);

    // check #5 Operator
    if (this->IsOperatorToken(k_sTempToken, cAddedChar))
        //return this->AssignToken(k_sTempToken, TokenTypes::OPERATOR);
        return this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::OPERATOR);

    else if (this->m_sType == TokenTypes::OPERATOR) 
        return TokenStatus::k_bFinished;
    
    // check #6: Value
    if (this->IsValueToken(k_sTempToken))
        return this->AssignToken(k_sTempToken, CheckIfValueIsKnown());
    
    // check #7: Identifier
    if (this->IsIdentifierToken(k_sTempToken))
        return this->AssignToken(k_sTempToken, TokenTypes::IDENTIFIER);

    // check #8: EOF
    if (cAddedChar == '\0')
        return this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::EOF_TOKEN);

    // if all checks failed -> Unknown token
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

size_t Token::GetLine() const noexcept
{
    return this->m_ulLine;
}

size_t Token::GetColumn() const noexcept
{
    return this->m_ulColumn;
}

bool Token::operator==(const Token& other) const
{
    return this->m_sType == other.m_sType && 
        this->m_sValue == other.m_sValue &&
        this->m_ulLine == other.m_ulLine &&
        this->m_ulColumn == other.m_ulColumn;
}

bool Token::AssignToken(
    const std::string &sTokenValue, 
    std::string_view sTokenType
) noexcept
{
    this->m_sValue = sTokenValue;
    this->m_sType = sTokenType;
    return TokenStatus::k_bUnfinished;
}

bool Token::AssignIfEmptyOrFinish(
    const std::string &sTokenValue, 
    std::string_view sTokenType
) noexcept
{
    // checking if there was already a token
    if (
        this->m_sValue.empty() || 
        this->m_sType == TokenTypes::UNKNOWN || 
        this->m_sType == sTokenType
    )
    {
        return AssignToken(sTokenValue, sTokenType);
    }
    return TokenStatus::k_bFinished;
}

std::string_view Token::CheckIfValueIsKnown() const noexcept
{
    /*if (cFirstLetter == '\"' && cLastLetter != '\"') return TokenTypes::UNKNOWN;
    
    if (cFirstLetter == '\'' && cLastLetter != '\'') return TokenTypes::UNKNOWN;*///strings and chars arent supported yet

    return TokenTypes::VALUE;
}

bool Token::CheckSoloLetters(
    const char k_cAddedChar, 
    const std::regex& k_regexCheck, 
    const std::string_view& k_sCurrentType
) const noexcept
{
    // A guarnteed stop will happen if the char is part of the regex, 
    // the token made isn't already it's type
    // and the current token isn't an edge case 
    // (currently only a string literal or char literal being made are edge cases).
    return (
        std::regex_match(std::string(1, k_cAddedChar), k_regexCheck) &&
        this->m_sType != k_sCurrentType
    ); 
    /*&& !(this->m_sType == TokenTypes::UNKNOWN && (this->m_sValue[0] == '\"' || this->m_sValue[0] == '\''))*/ //string and chars arent supported yet;
}

bool Token::IsCommentToken(const std::string &sToken) const
{
    // Line comments start with '#': they are comments until (but not including) '\n'.
    if (!sToken.empty() && sToken[0] == '#')
    {
        return sToken.find('\n') == std::string::npos;
    }

    // Block comments start with '/*' and end with '*/', possibly spanning multiple lines.
    // We consider ourselves "inside a comment" as long as:
    //  - the token starts with '/*', and
    //  - either we haven't seen '*/' yet,
    //    or we have seen it but there is nothing after it (the next char will close the token).
    if (sToken.size() >= 2 && sToken[0] == '/' && sToken[1] == '*')
    {
        const std::size_t endPos = sToken.find("*/", 2);
        if (endPos == std::string::npos)
        {
            // No closing '*/' yet -> still inside block comment.
            return true;
        }

        // We have a closing '*/'. Stay in the comment as long as there is
        // nothing after it; once more characters are appended, we should
        // stop treating it as a comment so the tokenizer can finish the token.
        return (endPos + 2 == sToken.size());
    }

    return false;
}

bool Token::IsWhiteSpaceToken(const std::string& sToken, char newChar) const
{
    return (
        std::regex_match(sToken, RegexPatterns::k_TriviaWhiteSpaces()) ||
        CheckSoloLetters(newChar, RegexPatterns::k_TriviaWhiteSpaces(), TokenTypes::TRIVIA)
    );
}

bool Token::IsPunctuationToken(const std::string& sToken, char newChar) const
{
    return (
        std::regex_match(sToken, RegexPatterns::k_Punctuation()) ||
        CheckSoloLetters(newChar, RegexPatterns::k_Punctuation(), TokenTypes::PUNCTUATION)
    );
}

bool Token::IsKeywordToken(const std::string& sToken) const
{
    return std::regex_match(sToken, RegexPatterns::k_Keyword());
}

bool Token::IsOperatorToken(const std::string& sToken, char newChar) const
{
    return (
        std::regex_match(sToken, RegexPatterns::k_Operator()) ||
        CheckSoloLetters(newChar, RegexPatterns::k_Operator(), TokenTypes::OPERATOR)
    );
}

bool Token::IsValueToken(const std::string& sToken) const
{
    return std::regex_match(sToken, RegexPatterns::k_Value());
}

bool Token::IsIdentifierToken(const std::string& sToken) const
{
    return std::regex_match(sToken, RegexPatterns::k_Identifier());
} 
