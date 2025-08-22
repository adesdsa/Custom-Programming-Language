#include "../../includes/tokenizer/token.h"

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
        std::regex_match(k_sTempToken, RegexPatterns::k_TriviaComments())
    )
    {
        return this->AssignToken(k_sTempToken, TokenTypes::UNFINISHED_TRIVIA);
    }
    else if (this->m_sType == TokenTypes::UNFINISHED_TRIVIA)
    {
        this->m_sType = TokenTypes::TRIVIA;
        return TokenStatus::k_bFinished;
    }

    if (
        std::regex_match(k_sTempToken, RegexPatterns::k_TriviaWhiteSpaces()) ||
        CheckSoloLetters(cAddedChar, RegexPatterns::k_TriviaWhiteSpaces(), TokenTypes::TRIVIA)
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
        CheckSoloLetters(cAddedChar, RegexPatterns::k_Punctuation(), TokenTypes::PUNCTUATION))
    {
        return this->AssignIfEmptyOrFinish(k_sTempToken, TokenTypes::PUNCTUATION);
    }
    else if (this->m_sType == TokenTypes::PUNCTUATION && this->m_sValue[0] != '\"' && this->m_sValue[0] != '\'')
        // if it's " or a ' it means we start a string/char litteral and we dont need to stop this token
    {
        return TokenStatus::k_bFinished;
    }

    if (std::regex_match(k_sTempToken, RegexPatterns::k_Keyword()))
    {
        return this->AssignToken(k_sTempToken, TokenTypes::KEYWORD);
    }

    if (
        std::regex_match(k_sTempToken, RegexPatterns::k_Operator()) ||
        CheckSoloLetters(cAddedChar, RegexPatterns::k_Operator(), TokenTypes::OPERATOR)
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
        return this->AssignToken(k_sTempToken, CheckIfValueIsKnown(k_sTempToken[0], cAddedChar));
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

const std::string Token::GetLine() const noexcept
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

std::string_view Token::CheckIfValueIsKnown(const char cFirstLetter, const char cLastLetter) noexcept
{
    if (cFirstLetter == '\"' && cLastLetter != '\"') return TokenTypes::UNKNOWN;
    
    if (cFirstLetter == '\'' && cLastLetter != '\'') return TokenTypes::UNKNOWN;

    return TokenTypes::VALUE;
}

bool Token::CheckSoloLetters(const char k_cAddedChar, const std::regex& k_regexCheck, const std::string_view& k_sCurrentType) noexcept
{
    // A guarnteed stop will happen if the char is part of the regex, the token made isn't already it's type
    // and the current token isn't an edge case (currently only a string literal or char literal being made are edge cases).
    return std::regex_match(std::string(1, k_cAddedChar), k_regexCheck) &&
        this->m_sType != k_sCurrentType &&
        !(this->m_sType == TokenTypes::UNKNOWN && (this->m_sValue[0] == '\"' || this->m_sValue[0] == '\''));
}
