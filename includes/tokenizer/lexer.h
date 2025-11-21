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

    // @return: single relevant token (none trivia) from source file
    std::optional<Token> Tokenize();

    // Checks If file has ended by reaching EOF.
    // @return: True -> File has ended (EOF), False -> Otherwise.
    bool IsFinishedReadingSRC() const noexcept;

    const EntityLogger& GetEntityLogger() const noexcept;

private:
    EntityLogger m_entityLogger;
    TokenFactory m_tokenFactory;
    TokenHandler m_tokenHandler;
};