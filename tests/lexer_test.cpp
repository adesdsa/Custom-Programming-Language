#include <gtest/gtest.h>

#include <sstream>

#include "tokenizer/lexer.h"


// Return the content of the file.
// @param path: Path to target file.
// @return: Content of the file.
const std::string& ReadFile(const std::string& path)
{
    std::ifstream file(path);

    if(!file.is_open())
    {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


// TEST: Empty source file.
// INPUT: Empty source file.
// EXPECTED: No tokens created.
TEST(LexerTest, EmptyInput) 
{
    Lexer lexer("../tests/inputs/test1.txt");
    lexer.Tokenize();

    EXPECT_EQ(ReadFile(k_sOutputFile), ReadFile("../tests/expected/test1.txt"));
}

// TEST: Only trivia tokens in source file.
// INPUT: Source file with different trivia tokens.
// EXPECTED: No tokens created.
TEST(LexerTest, OnlyTriviaTokens) 
{
    Lexer lexer("../tests/inputs/test2.txt");
    lexer.Tokenize();

    EXPECT_EQ(ReadFile(k_sOutputFile), ReadFile("../tests/expected/test2.txt"));
}