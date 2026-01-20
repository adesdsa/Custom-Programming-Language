#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include "includes/frontend/tokenizer/lexer.h"

namespace TestsUtils
{
    // Return the content of the file.
    // @param path: Path to target file.
    // @return: Content of the file.
    std::string ReadFile(const std::string& path);

    // Collect all tokens of the lexe into a vector.
    // @param lexer: Lexer to collect from.
    // @return: Vector of all lexer's tokens.
    std::vector<Token> CollectTokens(Lexer& lexer);

    // Make gtest asserions by comparing results and expected tokens.
    // @param results: Vector result of tokenizing all tokens by lexer.
    // @param expected: Vector of all expcted results of tokenizer.
    void CompareTokens(const std::vector<Token>& results, const std::vector<Token>& expected);
} 