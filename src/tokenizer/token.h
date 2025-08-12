#pragma once
#include <string>
#include <regex>
#include <vector>


// TODO: check the maximum scope of the vector
const std::vector<const std::string> k_tokenTypevector =
{ "PUNCTUATION", "KEYWORD", "VALUE", "OPERATOR",
"IDENTIFIER", "TRIVIA", "EOF", "UNKNOWN" };

class Token
{
public:
    Token(size_t ulLine) noexcept;
     // Tries to append the new character to the Token unless it's a char for
     // the next token than it defines the token type.
     // @param cAddedChar: The character we add to the token right now.
     // @return: True => The token is finished, False => Otherwise.
    bool Append(char cAddedChar) noexcept;

    std::string GetValue() const noexcept; 
    std::string GetType() const noexcept;
    std::string GetLine() const noexcept;

private:
    std::string m_sValue;
    std::string m_sType;
    size_t m_ulLine;

};
