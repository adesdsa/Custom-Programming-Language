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

    // Get every token from the source file and push it to the stream.
    // @return: Relevant token => Reference to the token. Otherwise => nullopt. 
    std::optional<Token> Tokenize();

    // Checks if file is in EOF.
    // @return: True => , False => .
    bool IsFinishedReadingSRC() const noexcept;

    const EntityLogger& GetEntityLogger();

private:
    TokenFactory m_tokenFactory;
    TokenHandler m_tokenHandler;
    EntityLogger m_entityLogger;
};