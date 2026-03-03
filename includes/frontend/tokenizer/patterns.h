#pragma once

#include <regex>
#include <iostream>
#include <string>

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
                return std::regex(R"(->|[:,\(\)\{\};#])", std::regex::optimize);
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
                    R"(\b(and|or|not|xor|break|continue|return|if|elif|else|for|while|pass|sizeof|const|readonly|void|bool|unsigned|int|double|global|var|let|export|import|from)\b)", // MUST BE IN A SINGLE LINE!
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
                    R"(\b(true|false)\b|-?\d+\.\d+|-?\d+)",
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

    inline const std::regex& k_Operator()
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(
                    R"(//|==|!=|<=|>=|[+\-*/%=<>!])",
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

    inline const std::regex& k_TriviaComments() 
    {
        static const std::regex r([]()
        {
            try 
            {
                return std::regex(R"(#[^\n]*|/\*(?:[^*]|\*+[^*/])*?\*/?|/\*[^*]*$)", std::regex::optimize);
            } catch (const std::regex_error& e) 
            {
                std::cerr << "Regex k_Trivia failed: " << e.what() << std::endl;
                throw;
            }
        }());
        return r;
    }

    inline const std::regex& k_TriviaWhiteSpaces()
    {
        static const std::regex r([]()
            {
                try
                {
                    return std::regex(R"(\s+)", std::regex::optimize);
                }
                catch (const std::regex_error& e)
                {
                    std::cerr << "Regex k_Trivia failed: " << e.what() << std::endl;
                    throw;
                }
            }());
        return r;
    }


}
