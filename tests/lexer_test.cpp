#include <gtest/gtest.h>

#include "tokenizer/lexer.h"
#include "utils/tests_utils.h"

// TEST: Empty source file
TEST(LexerTest, EmptyInput) 
{
    std::vector<Token> expected;

    Lexer lexer("../tests/inputs/test1.txt");
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}

// TEST: Only trivia tokens in source file
TEST(LexerTest, OnlyTriviaTokens) 
{
    std::vector<Token> expected;

    Lexer lexer("../tests/inputs/test2.txt");
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);
    
    TestsUtils::CompareTokens(results, expected);
}

// TEST: A small code example
TEST(LexerTest, SmallCodeExample) 
{
    Lexer lexer("../tests/inputs/test3.txt");

    std::vector<Token> expected = {
        // Line 5
        Token("Add", TokenTypes::IDENTIFIER,5,lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION,5,lexer.GetEntityLogger()),
        Token("let",TokenTypes::KEYWORD,5,lexer.GetEntityLogger()),
        Token("a",TokenTypes::IDENTIFIER,5,lexer.GetEntityLogger()),
        Token(":",TokenTypes::PUNCTUATION,5,lexer.GetEntityLogger()),
        Token("int",TokenTypes::KEYWORD,5,lexer.GetEntityLogger()),
        Token(",",TokenTypes::PUNCTUATION,5,lexer.GetEntityLogger()),
        Token("let",TokenTypes::KEYWORD,5,lexer.GetEntityLogger()),
        Token("b",TokenTypes::IDENTIFIER,5,lexer.GetEntityLogger()),
        Token(":",TokenTypes::PUNCTUATION,5,lexer.GetEntityLogger()),
        Token("int",TokenTypes::KEYWORD,5,lexer.GetEntityLogger()),
        Token(")",TokenTypes::PUNCTUATION,5,lexer.GetEntityLogger()),
        Token("->",TokenTypes::PUNCTUATION,5,lexer.GetEntityLogger()),
        Token("void",TokenTypes::KEYWORD,5,lexer.GetEntityLogger()),

        // Line 6
        Token("{",TokenTypes::PUNCTUATION,6,lexer.GetEntityLogger()),

        // Line 7
        Token("print",TokenTypes::IDENTIFIER,7,lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION,7,lexer.GetEntityLogger()),
        Token("\"Hello, this is a test!\"",TokenTypes::VALUE,7,lexer.GetEntityLogger()),
        Token(")",TokenTypes::PUNCTUATION,7,lexer.GetEntityLogger()),
        Token(";",TokenTypes::PUNCTUATION,7,lexer.GetEntityLogger()),

        // Line 8
        Token("}",TokenTypes::PUNCTUATION,8,lexer.GetEntityLogger()),
    };

    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}

// TEST: Full program example
TEST(LexerTest, FullProgram) 
{
    Lexer lexer("../tests/inputs/test4.txt");

    std::vector<Token> expected = {
        // Line 1
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 1, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 1, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 1, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 1, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 1, lexer.GetEntityLogger()),
        Token(",", TokenTypes::PUNCTUATION, 1, lexer.GetEntityLogger()),
        Token("high", TokenTypes::IDENTIFIER, 1, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 1, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 1, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 1, lexer.GetEntityLogger()),
        Token("->", TokenTypes::PUNCTUATION, 1, lexer.GetEntityLogger()),
        Token("void", TokenTypes::KEYWORD, 1, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 1, lexer.GetEntityLogger()),

        // Line 3
        Token("main", TokenTypes::IDENTIFIER, 3, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 3, lexer.GetEntityLogger()),
        Token("void", TokenTypes::KEYWORD, 3, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 3, lexer.GetEntityLogger()),
        Token("->", TokenTypes::PUNCTUATION, 3, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 3, lexer.GetEntityLogger()),

        // Line 4
        Token("{", TokenTypes::PUNCTUATION, 4, lexer.GetEntityLogger()),

        // Line 5
        Token("let", TokenTypes::KEYWORD, 5, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 5, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 5, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 5, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 5, lexer.GetEntityLogger()),
        Token("get_int", TokenTypes::IDENTIFIER, 5, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 5, lexer.GetEntityLogger()),
        Token("\"Enter low number in range\"", TokenTypes::VALUE, 5, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 5, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 5, lexer.GetEntityLogger()),

        // Line 6
        Token("let", TokenTypes::KEYWORD, 6, lexer.GetEntityLogger()),
        Token("high", TokenTypes::IDENTIFIER, 6, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 6, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 6, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 6, lexer.GetEntityLogger()),
        Token("get_int", TokenTypes::IDENTIFIER, 6, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 6, lexer.GetEntityLogger()),
        Token("\"Enter high number in range\"", TokenTypes::VALUE, 6, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 6, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 6, lexer.GetEntityLogger()),

        // Line 8
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 8, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 8, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 8, lexer.GetEntityLogger()),
        Token(",", TokenTypes::PUNCTUATION, 8, lexer.GetEntityLogger()),
        Token("high", TokenTypes::IDENTIFIER, 8, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 8, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 8, lexer.GetEntityLogger()),

        // Line 9
        Token("return", TokenTypes::KEYWORD, 9, lexer.GetEntityLogger()),
        Token("0", TokenTypes::VALUE, 9, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 9, lexer.GetEntityLogger()),

        // Line 10
        Token("}", TokenTypes::PUNCTUATION, 10, lexer.GetEntityLogger()),

        // Line 13
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 13, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 13, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 13, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 13, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 13, lexer.GetEntityLogger()),
        Token(",", TokenTypes::PUNCTUATION, 13, lexer.GetEntityLogger()),
        Token("high", TokenTypes::IDENTIFIER, 13, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 13, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 13, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 13, lexer.GetEntityLogger()),
        Token("->", TokenTypes::PUNCTUATION, 13, lexer.GetEntityLogger()),
        Token("void", TokenTypes::KEYWORD, 13, lexer.GetEntityLogger()),

        // Line 14
        Token("{", TokenTypes::PUNCTUATION, 14, lexer.GetEntityLogger()),

        // Line 15
        Token("while", TokenTypes::KEYWORD, 15, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 15, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 15, lexer.GetEntityLogger()),
        Token("<", TokenTypes::OPERATOR, 15, lexer.GetEntityLogger()),
        Token("high", TokenTypes::IDENTIFIER, 15, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 15, lexer.GetEntityLogger()),

        // Line 16
        Token("{", TokenTypes::PUNCTUATION, 16, lexer.GetEntityLogger()),

        // Line 17
        Token("let", TokenTypes::KEYWORD, 17, lexer.GetEntityLogger()),
        Token("is_prime", TokenTypes::IDENTIFIER, 17, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 17, lexer.GetEntityLogger()),
        Token("bool", TokenTypes::KEYWORD, 17, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 17, lexer.GetEntityLogger()),
        Token("true", TokenTypes::KEYWORD, 17, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 17, lexer.GetEntityLogger()),

        // Line 20
        Token("if", TokenTypes::KEYWORD, 20, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 20, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 20, lexer.GetEntityLogger()),
        Token("==", TokenTypes::OPERATOR, 20, lexer.GetEntityLogger()),
        Token("0", TokenTypes::VALUE, 20, lexer.GetEntityLogger()),
        Token("or", TokenTypes::KEYWORD, 20, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 20, lexer.GetEntityLogger()),
        Token("==", TokenTypes::OPERATOR, 20, lexer.GetEntityLogger()),
        Token("1", TokenTypes::VALUE, 20, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 20, lexer.GetEntityLogger()),

        // Line 21
        Token("{", TokenTypes::PUNCTUATION, 21, lexer.GetEntityLogger()),

        // Line 22
        Token("is_prime", TokenTypes::IDENTIFIER, 22, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 22, lexer.GetEntityLogger()),
        Token("false", TokenTypes::KEYWORD, 22, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 22, lexer.GetEntityLogger()),

        // Line 23
        Token("}", TokenTypes::PUNCTUATION, 23, lexer.GetEntityLogger()),

        // Line 25
        Token("for", TokenTypes::KEYWORD, 25, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 25, lexer.GetEntityLogger()),
        Token("let", TokenTypes::KEYWORD, 25, lexer.GetEntityLogger()),
        Token("i", TokenTypes::IDENTIFIER, 25, lexer.GetEntityLogger()),
        Token(":", TokenTypes::PUNCTUATION, 25, lexer.GetEntityLogger()),
        Token("int", TokenTypes::KEYWORD, 25, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 25, lexer.GetEntityLogger()),
        Token("2", TokenTypes::VALUE, 25, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 25, lexer.GetEntityLogger()),
        Token("i", TokenTypes::IDENTIFIER, 25, lexer.GetEntityLogger()),
        Token("<=", TokenTypes::OPERATOR, 25, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 25, lexer.GetEntityLogger()),
        Token("//", TokenTypes::OPERATOR, 25, lexer.GetEntityLogger()),
        Token("2", TokenTypes::VALUE, 25, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 25, lexer.GetEntityLogger()),
        Token("i", TokenTypes::IDENTIFIER, 25, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 25, lexer.GetEntityLogger()),
        Token("i", TokenTypes::IDENTIFIER, 25, lexer.GetEntityLogger()),
        Token("+", TokenTypes::OPERATOR, 25, lexer.GetEntityLogger()),
        Token("1", TokenTypes::VALUE, 25, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 25, lexer.GetEntityLogger()),
        
        // Line 26
        Token("{", TokenTypes::PUNCTUATION, 26, lexer.GetEntityLogger()),

        // Line 27
        Token("if", TokenTypes::KEYWORD, 27, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 27, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 27, lexer.GetEntityLogger()),
        Token("%", TokenTypes::OPERATOR, 27, lexer.GetEntityLogger()),
        Token("i", TokenTypes::IDENTIFIER, 27, lexer.GetEntityLogger()),
        Token("==", TokenTypes::OPERATOR, 27, lexer.GetEntityLogger()),
        Token("0", TokenTypes::VALUE, 27, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 27, lexer.GetEntityLogger()),

        // Line 28
        Token("{", TokenTypes::PUNCTUATION, 28, lexer.GetEntityLogger()),

        // Line 29
        Token("is_prime", TokenTypes::IDENTIFIER, 29, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 29, lexer.GetEntityLogger()),
        Token("false", TokenTypes::KEYWORD, 29, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 29, lexer.GetEntityLogger()),
        
        // Line 30
        Token("break", TokenTypes::KEYWORD, 30, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 30, lexer.GetEntityLogger()),

        // Line 31
        Token("}", TokenTypes::PUNCTUATION, 31, lexer.GetEntityLogger()),

        // Line 32
        Token("}", TokenTypes::PUNCTUATION, 32, lexer.GetEntityLogger()),

        // Line 34
        Token("if", TokenTypes::KEYWORD, 34, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 34, lexer.GetEntityLogger()),
        Token("is_prime", TokenTypes::IDENTIFIER, 34, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 34, lexer.GetEntityLogger()),

        // Line 35
        Token("{", TokenTypes::PUNCTUATION, 35, lexer.GetEntityLogger()),

        // Line 36
        Token("print", TokenTypes::IDENTIFIER, 36, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 36, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 36, lexer.GetEntityLogger()),
        Token("+", TokenTypes::OPERATOR, 36, lexer.GetEntityLogger()),
        Token("\" , \"", TokenTypes::VALUE, 36, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 36, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 36, lexer.GetEntityLogger()),

        // Line 37
        Token("}", TokenTypes::PUNCTUATION, 37, lexer.GetEntityLogger()),

        // Line 39
        Token("low", TokenTypes::IDENTIFIER, 39, lexer.GetEntityLogger()),
        Token("=", TokenTypes::OPERATOR, 39, lexer.GetEntityLogger()),
        Token("low", TokenTypes::IDENTIFIER, 39, lexer.GetEntityLogger()),
        Token("+", TokenTypes::OPERATOR, 39, lexer.GetEntityLogger()),
        Token("1", TokenTypes::VALUE, 39, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 39, lexer.GetEntityLogger()),

        // Line 40
        Token("}", TokenTypes::PUNCTUATION, 40, lexer.GetEntityLogger()),

        // Line 41
        Token("println", TokenTypes::IDENTIFIER, 41, lexer.GetEntityLogger()),
        Token("(", TokenTypes::PUNCTUATION, 41, lexer.GetEntityLogger()),
        Token(")", TokenTypes::PUNCTUATION, 41, lexer.GetEntityLogger()),
        Token(";", TokenTypes::PUNCTUATION, 41, lexer.GetEntityLogger()),

        // Line 42
        Token("}", TokenTypes::PUNCTUATION, 41, lexer.GetEntityLogger()),
    };

    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}