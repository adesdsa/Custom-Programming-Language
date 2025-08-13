#pragma once

#include "token_factory.h"
#include "token_handler.h"

constexpr const char* k_sInputFile = "tokens_in.txt";

// Responsible for managing all Token entityies to convert src to stream of tokens
class Lexer
{
public:
    Lexer();
    ~Lexer() = default;

    // Get every token from the source file and push it to the stream
    // @return: None.
    void Tokenize();


private:
    TokenFactory m_tokenFactory;
    TokenHandler m_tokenHandler;

    // Opens the input file stream.
    // @return: Return the opened file.
    static std::ifstream OpenInputFile()
    {
        std::ifstream file(k_sInputFile);

        if (!file) 
        {
            throw std::runtime_error(
                std::string(FUNCTION_THROWER) + 
                k_sFileFailedToOpenError + 
                k_sInputFile);
        }

        return file;
    }
};