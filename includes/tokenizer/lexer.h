#pragma once

#include "token_factory.h"
#include "token_handler.h"
#include "../logger/entity_logger.h"

constexpr const char* k_sInputFile = "../../src/tokenizer/tokens_in.txt"; // "tokens_in.txt"

// Responsible for managing all Token entityies to convert src to stream of tokens
class Lexer
{
public:
    Lexer();

    Lexer(const std::string& sInputFile);
    
    ~Lexer() = default;

    // Create a new token from stream.
    // @return: New token or nullopt if token is irrelevant.  
    std::optional<std::reference_wrapper<const Token>>Tokenize();

private:
    TokenFactory m_tokenFactory;
    TokenHandler m_tokenHandler;
    EntityLogger m_entityLogger;

    // Opens the input file stream.
    // @return: Return the opened file.
    static std::ifstream OpenInputFile(const std::string& sInputFile);
};