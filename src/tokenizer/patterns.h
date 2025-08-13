#pragma once

#include <regex>
#include <iostream>
#include <string>

// TODO: Change regexs according to the language syntax
// Group of all the regex patterns supported by lexer
namespace RegexPatterns
{
    /////////////////////////////////////////////////////////////////////////+
    // General Documentation For Rest Of The Code   ||                       |
    //                                              \/                       | 
    // Code: inline const std::regex& patternName()                          |       
    // Explenation: Create function that return a const regex                |
    //-----------------------------------------------------------------------|
    // Code: static const std::regex r([]()                                  |
    // Explenation: Create a static variable of type const regex meaning     |
    // I will be created only once into memory. []( Create a lambda          |
    // constructor that return the pattern or throw error                    |
    // ----------------------------------------------------------------------|
    // Summery: Function call will be replaced with appropriate pattern      |
    /////////////////////////////////////////////////////////////////////////+

    inline const std::regex& k_Identifier() 
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(R"(^[A-Za-z_][A-Za-z0-9_]*$)", std::regex::optimize);
            } 
            catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_Identifier failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }

    inline const std::regex& k_Punctuation() 
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(R"(::|->|##|;|,|\.|:|\?|#|\{|\}|\(|\)|\[|\])", std::regex::optimize);
            } 
            catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_Punctuation failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }

    inline const std::regex& k_Keyword() 
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(
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
            } 
            catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_Keyword failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }

    inline const std::regex& k_Value() 
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(
                    std::string(R"(\b(?:true|false|nullptr|NULL)\b)") + 
                    R"(|(?:0b[01]+|0x[0-9A-Fa-f]+|0[0-7]+|[0-9]+)(?:[uU]?(?:l{1,2}|L{1,2})?)?)" +
                    R"(|\d+\.\d*(?:[eE][+-]?\d+)?[fFlL]?)" +
                    R"(|\d+(?:[eE][+-]?\d+)[fFlL]?)" + 
                    R"(|'(?:\\.|[^\\'])')" + 
                    R"(|"(?:\\.|[^\\"])*")",
                    std::regex::optimize
                );
            } 
            catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_Value failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }

    inline const std::regex& k_KeywordOperator() 
    {
        static const std::regex r([]()
        {
            try 
            
            {
                return std::regex(R"(\b(?:sizeof|alignof|typeid|decltype|new|delete|co_await)\b)", std::regex::optimize);
            } catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_KeywordOperator failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }

    inline const std::regex& k_KeywordSymbol() 
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(
                    R"((?:\+\+|--|==|!=|<=|>=|&&|\|\||\+=|-=|\*=|/=|%=|\^=|&=|\|=|<<=|>>=|<<|>>|->\*|->|\.|\.\*|::|[+\-*/%^&|~!<>=?:]))",
                    std::regex::optimize
                );
            } 
            catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_KeywordSymbol failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }

    inline const std::regex& k_Trivia() 
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(R"(?:\s+|//[^\n]*|/\*[\s\S]*?\*/)", std::regex::optimize);
            } catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_Trivia failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }
}
