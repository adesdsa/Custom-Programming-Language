#include <gtest/gtest.h>

#include <optional>
#include <vector>

#include "tokenizer/lexer.h"
#include "utils/file_utils.h"


// TEST: Empty source file.
// INPUT: Empty source file.
// EXPECTED: No tokens created.
TEST(LexerTest, EmptyInput) 
{
    Lexer lexer("/inputs/test1.txt");

    std::vector<std::optional<Token>> results;
    std::vector<std::optional<Token>> expected;
    
    while(!lexer.IsFinishedReadingSRC())
    {
        results.push_back(lexer.Tokenize());
    }

    ASSERT_EQ(results.size(), expected.size()) << "Result and expected token counts differ.";

    for (size_t i = 0; i < results.size(); i++) 
    {
        if (!expected[i].has_value()) 
        {
            EXPECT_FALSE(results[i].has_value()) << "Expected nullopt at index " << i;
        } 
        else 
        {
            ASSERT_TRUE(results[i].has_value()) << "Expected a Token at index " << i;
            EXPECT_EQ(results[i].value(), expected[i].value()) << "Mismatch at token index " << i;
        }
    }
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