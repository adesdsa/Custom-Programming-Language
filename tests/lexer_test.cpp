#include <gtest/gtest.h>

#include "tokenizer/lexer.h"
#include "utils/file_utils.h"


// TEST: Empty source file.
// INPUT: Empty source file.
// EXPECTED: No tokens created.
TEST(LexerTest, EmptyInput) 
{
}

// TEST: Only trivia tokens in source file.
// INPUT: Source file with different trivia tokens.
// EXPECTED: No tokens created.
TEST(LexerTest, OnlyTriviaTokens) 
{
}

// TEST: A small code example (code with trivia)
// INPUT: Source file with small code example.
// EXPECTED: Tokens of the code and delete trivia part.
TEST(LexerTest, SmallCodeExample) 
{
}