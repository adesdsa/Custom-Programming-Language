#include <gtest/gtest.h>

#include "tokenizer/lexer.h"
#include "utils/tests_utils.h"

/// TODO: Add forth test for full program (see drive for example)

// TEST: Empty source file.
// INPUT: Empty source file.
// EXPECTED: No tokens created.
TEST(LexerTest, EmptyInput) 
{
    std::vector<Token> expected;

    Lexer lexer("/tests/inputs/test1.txt");
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}

// TEST: Only trivia tokens in source file.
// INPUT: Source file with different trivia tokens.
// EXPECTED: No tokens created.
TEST(LexerTest, OnlyTriviaTokens) 
{
    std::vector<Token> expected;

    Lexer lexer("/tests/inputs/test2.txt");
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);
    
    TestsUtils::CompareTokens(results, expected);
}

// TEST: A small code example (code with trivia)
// INPUT: Source file with small code example.
// EXPECTED: Tokens of the code and delete trivia part.
TEST(LexerTest, SmallCodeExample) 
{
    std::vector<Token> expected = {
        Token("void",TokenTypes::KEYWORD,7),
        Token("Add", TokenTypes::IDENTIFIER,7),
        Token("(", TokenTypes::PUNCTUATION,7),
        Token("int",TokenTypes::KEYWORD,7),
        Token("a",TokenTypes::IDENTIFIER,7),
        Token(",",TokenTypes::PUNCTUATION,7),
        Token("int",TokenTypes::KEYWORD,7),
        Token("b",TokenTypes::IDENTIFIER,7),
        Token(")",TokenTypes::PUNCTUATION,7),
        Token("{",TokenTypes::PUNCTUATION,8),
        Token("print",TokenTypes::IDENTIFIER,9),
        Token("(", TokenTypes::PUNCTUATION,9),
        Token("\"Hello, this is a test!\"",TokenTypes::VALUE,9),
        Token(")",TokenTypes::PUNCTUATION,9),
        Token(";",TokenTypes::PUNCTUATION,9),
        Token("}",TokenTypes::PUNCTUATION,10),
    };

    Lexer lexer("/tests/inputs/test3.txt");
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}