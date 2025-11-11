#pragma once

#include "token_factory.h"
#include "token_handler.h"
#include "../logger/entity_logger.h"

// Responsible for managing all Token entityies to convert src to stream of tokens
class Lexer
{
public:
    Lexer(const std::string& sFilePath);
    
    ~Lexer() = default;

    // Get every token from the source file and push it to the stream.
    // @return: Relevant token => Reference to the token. Otherwise => nullopt. 
    std::optional<Token> Tokenize() noexcept;

    // Checks If file has ended by reaching EOF.
    // @return: True => File has ended (EOF), False => Otherwise.
    bool IsFinishedReadingSRC() const noexcept;

    const EntityLogger& GetEntityLogger() const noexcept;

private:
    TokenFactory m_tokenFactory;
    TokenHandler m_tokenHandler;
    EntityLogger m_entityLogger;
};