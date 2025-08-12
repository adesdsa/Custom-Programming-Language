#pragma once

#include <regex>

// TODO: Change regexs according to the language syntax
// Group of all the regex patterns supported by lexer
namespace RegexPatterns
{
    inline const std::regex k_Identifier(
        R"(^[A-Za-z_][A-Za-z0-9_]*$)", 
        std::regex::optimize
    );

    inline const std::regex k_Punctuation(
        R"(::|->|##|;|,|\.|:|\?|#|\{|\}|\(|\)|\[|\])",
        std::regex::optimize
    );

    // All supported keyword of the language
    inline const std::regex k_Keyword(
        std::string(R"(\b(?:alignas|alignof|and|and_eq|asm|auto|)") +
        R"(bitand|bitor|bool|break|case|catch|char|char8_t|)" + 
        R"(char16_t|char32_t|class|compl|concept|const|consteval|)" + 
        R"(constexpr|constinit|const_cast|continue|co_await|)" + 
        R"(co_return|co_yield|decltype|default|delete|do|)" + 
        R"(double|dynamic_cast|else|enum|explicit|export|)" + 
        R"(extern|false|float|for|friend|goto|if|inline|int|)" +
        R"(long|mutable|namespace|new|noexcept|not|not_eq|)" +
        R"(nullptr|operator|or|or_eq|private|protected|)" +
        R"(public|reflexpr|register|reinterpret_cast|requires|)" +
        R"(return|short|signed|sizeof|static|static_assert|)" +
        R"(static_cast|struct|switch|synchronized|template|)" +
        R"(this|thread_local|throw|true|try|typedef|typeid|)" +
        R"(typename|union|unsigned|using|virtual|void|)" +
        R"(volatile|wchar_t|while|xor|xor_eq)\b)", 
        std::regex::optimize
    );

    inline const std::regex k_ValuePattern(
        std::string(R"(\b(?:true|false|nullptr|NULL)\b)") + 
        R"(|(?:0b[01]+|0x[0-9A-Fa-f]+|0[0-7]+|[0-9]+)(?:[uU]?(?:l{1,2}|L{1,2})?)?)" +
        R"(|\d+\.\d*(?:[eE][+-]?\d+)?[fFlL]?)" +
        R"(|\d+(?:[eE][+-]?\d+)[fFlL]?)" + 
        R"(|'(?:\\.|[^\\'])')" + 
        R"(|"(?:\\.|[^\\"])*")",
        std::regex::optimize
    );

    inline const std::regex k_rKeywordOperator(
        R"(\b(?:sizeof|alignof|typeid|decltype|new|delete|co_await)\b")", 
        std::regex::optimize
    );


    inline const std::regex k_KeywordSymbol(
        R"(R"(|(?:\+\+|--|==|!=|<=|>=|&&|\|\||\+=|-=|\*=|/=|%=|\^=|&=|\|=|<<=|>>=|<<|>>|->\*|->|\.|\.\*|::|[+\-*/%^&|~!<>=?:]))", 
        std::regex::optimize
    );

    inline const std::regex k_Trivia(
        R"(? : \s + |//[^\n]*|/\*[\s\S]*?\*/)",
        std::regex::optimize
    );
}