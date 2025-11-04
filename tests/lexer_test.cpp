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
    Lexer lexer("/tests/inputs/test1.txt");

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
    Lexer lexer("/tests/inputs/test2.txt");

    std::vector<std::optional<Token>> results;
    std::vector<std::optional<Token>> expected;
    
    while(!lexer.IsFinishedReadingSRC())
    {
        auto token = lexer.Tokenize();

        if(token != std::nullopt)
        {
            results.push_back(token);
        }
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

// TEST: A small code example (code with trivia)
// INPUT: Source file with small code example.
// EXPECTED: Tokens of the code and delete trivia part.
TEST(LexerTest, SmallCodeExample) 
{
    Lexer lexer("/tests/inputs/test3.txt");

    std::vector<std::optional<Token>> results;
    std::vector<std::optional<Token>> expected = {
        Token("int","KEYWORD",7),
        Token("Add", "IDENTIFIER",7),
        Token("(", "PUNCTUATION",7),
        Token("int","KEYWORD",7),
        Token("a","IDENTIFIER",7),
        Token(",","PUNCTUATION",7),
        Token("int","KEYWORD",7),
        Token("b","IDENTIFIER",7),
        Token(")","PUNCTUATION",7),
        Token("{","PUNCTUATION",8),
        Token("print","IDENTIFIER",9),
        Token("(", "PUNCTUATION",9),
        Token("\"Hello, this is a test!\"","VALUE",9),
        Token(")","PUNCTUATION",9),
        Token(";","PUNCTUATION",9),
        Token("}","PUNCTUATION",10),
    };
    
    while(!lexer.IsFinishedReadingSRC())
    {
        auto token = lexer.Tokenize();

        if(token != std::nullopt)   
        {
            results.push_back(token);
        }
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