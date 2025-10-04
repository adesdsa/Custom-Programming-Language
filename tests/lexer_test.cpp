#include <gtest/gtest.h>

#include "tokenizer/lexer.h"
#include "utils/file_utils.h"


// TEST: Empty source file.
// INPUT: Empty source file.
// EXPECTED: No tokens created.
TEST(LexerTest, EmptyInput) 
{
    Lexer lexer("/tests/inputs/test1.txt");
    lexer.Tokenize();

    EXPECT_EQ(FileUtils::ReadFile(k_sOutputFile), FileUtils::ReadFile("/tests/expected/test1.txt"));
}

// TEST: Only trivia tokens in source file.
// INPUT: Source file with different trivia tokens.
// EXPECTED: No tokens created.
TEST(LexerTest, OnlyTriviaTokens) 
{
    Lexer lexer("/tests/inputs/test2.txt");
    lexer.Tokenize();

    EXPECT_EQ(FileUtils::ReadFile(k_sOutputFile), FileUtils::ReadFile("/tests/expected/test2.txt"));
}

// TEST: A small code example (code with trivia)
// INPUT: Source file with small code example.
// EXPECTED: Tokens of the code and delete trivia part.
TEST(LexerTest, SmallCodeExample) 
{
    Lexer lexer("/tests/inputs/test3.txt");
    lexer.Tokenize();

    EXPECT_EQ(FileUtils::ReadFile(k_sOutputFile), FileUtils::ReadFile("/tests/expected/test3.txt"));
}