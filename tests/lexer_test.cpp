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
        Token("Add", TokenTypes::IDENTIFIER,5),
        Token("(", TokenTypes::PUNCTUATION,5),
        Token("let",TokenTypes::KEYWORD,5),
        Token("a",TokenTypes::IDENTIFIER,5),
        Token(":",TokenTypes::PUNCTUATION,5),
        Token("int",TokenTypes::KEYWORD,5),
        Token(",",TokenTypes::PUNCTUATION,5),
        Token("let",TokenTypes::KEYWORD,5),
        Token("b",TokenTypes::IDENTIFIER,5),
        Token(":",TokenTypes::PUNCTUATION,5),
        Token("int",TokenTypes::KEYWORD,5),
        Token(")",TokenTypes::PUNCTUATION,5),
        Token("->",TokenTypes::PUNCTUATION,5),
        Token("void",TokenTypes::KEYWORD,5),

        // Line 6
        Token("{",TokenTypes::PUNCTUATION,6),

        // Line 7
        Token("println",TokenTypes::IDENTIFIER,7),
        Token("(", TokenTypes::PUNCTUATION,7),
        Token("3", TokenTypes::VALUE,7),
        Token("+", TokenTypes::OPERATOR,7),
        Token("2", TokenTypes::VALUE,7),
        Token("==", TokenTypes::OPERATOR,7),
        Token("5", TokenTypes::VALUE,7),
        Token(")",TokenTypes::PUNCTUATION,7),
        Token(";",TokenTypes::PUNCTUATION,7),

        // Line 8
        Token("}",TokenTypes::PUNCTUATION,8),
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
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 1),
        Token("(", TokenTypes::PUNCTUATION, 1),
        Token("low", TokenTypes::IDENTIFIER, 1),
        Token(":", TokenTypes::PUNCTUATION, 1),
        Token("int", TokenTypes::KEYWORD, 1),
        Token(",", TokenTypes::PUNCTUATION, 1),
        Token("high", TokenTypes::IDENTIFIER, 1),
        Token(":", TokenTypes::PUNCTUATION, 1),
        Token("int", TokenTypes::KEYWORD, 1),
        Token(")", TokenTypes::PUNCTUATION, 1),
        Token("->", TokenTypes::PUNCTUATION, 1),
        Token("void", TokenTypes::KEYWORD, 1),
        Token(";", TokenTypes::PUNCTUATION, 1),

        // Line 3
        Token("main", TokenTypes::IDENTIFIER, 3),
        Token("(", TokenTypes::PUNCTUATION, 3),
        Token("void", TokenTypes::KEYWORD, 3),
        Token(")", TokenTypes::PUNCTUATION, 3),
        Token("->", TokenTypes::PUNCTUATION, 3),
        Token("int", TokenTypes::KEYWORD, 3),

        // Line 4
        Token("{", TokenTypes::PUNCTUATION, 4),

        // Line 5
        Token("let", TokenTypes::KEYWORD, 5),
        Token("low", TokenTypes::IDENTIFIER, 5),
        Token(":", TokenTypes::PUNCTUATION, 5),
        Token("int", TokenTypes::KEYWORD, 5),
        Token("=", TokenTypes::OPERATOR, 5),
        Token("get_int", TokenTypes::IDENTIFIER, 5),
        Token("(", TokenTypes::PUNCTUATION, 5),
        Token(")", TokenTypes::PUNCTUATION, 5),
        Token(";", TokenTypes::PUNCTUATION, 5),

        // Line 6
        Token("let", TokenTypes::KEYWORD, 6),
        Token("high", TokenTypes::IDENTIFIER, 6),
        Token(":", TokenTypes::PUNCTUATION, 6),
        Token("int", TokenTypes::KEYWORD, 6),
        Token("=", TokenTypes::OPERATOR, 6),
        Token("get_int", TokenTypes::IDENTIFIER, 6),
        Token("(", TokenTypes::PUNCTUATION, 6),
        Token(")", TokenTypes::PUNCTUATION, 6),
        Token(";", TokenTypes::PUNCTUATION, 6),

        // Line 8
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 8),
        Token("(", TokenTypes::PUNCTUATION, 8),
        Token("low", TokenTypes::IDENTIFIER, 8),
        Token(",", TokenTypes::PUNCTUATION, 8),
        Token("high", TokenTypes::IDENTIFIER, 8),
        Token(")", TokenTypes::PUNCTUATION, 8),
        Token(";", TokenTypes::PUNCTUATION, 8),

        // Line 9
        Token("return", TokenTypes::KEYWORD, 9),
        Token("0", TokenTypes::VALUE, 9),
        Token(";", TokenTypes::PUNCTUATION, 9),

        // Line 10
        Token("}", TokenTypes::PUNCTUATION, 10),

        // Line 13
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 13),
        Token("(", TokenTypes::PUNCTUATION, 13),
        Token("low", TokenTypes::IDENTIFIER, 13),
        Token(":", TokenTypes::PUNCTUATION, 13),
        Token("int", TokenTypes::KEYWORD, 13),
        Token(",", TokenTypes::PUNCTUATION, 13),
        Token("high", TokenTypes::IDENTIFIER, 13),
        Token(":", TokenTypes::PUNCTUATION, 13),
        Token("int", TokenTypes::KEYWORD, 13),
        Token(")", TokenTypes::PUNCTUATION, 13),
        Token("->", TokenTypes::PUNCTUATION, 13),
        Token("void", TokenTypes::KEYWORD, 13),

        // Line 14
        Token("{", TokenTypes::PUNCTUATION, 14),

        // Line 15
        Token("while", TokenTypes::KEYWORD, 15),
        Token("(", TokenTypes::PUNCTUATION, 15),
        Token("low", TokenTypes::IDENTIFIER, 15),
        Token("<", TokenTypes::OPERATOR, 15),
        Token("high", TokenTypes::IDENTIFIER, 15),
        Token(")", TokenTypes::PUNCTUATION, 15),

        // Line 16
        Token("{", TokenTypes::PUNCTUATION, 16),

        // Line 17
        Token("let", TokenTypes::KEYWORD, 17),
        Token("is_prime", TokenTypes::IDENTIFIER, 17),
        Token(":", TokenTypes::PUNCTUATION, 17),
        Token("bool", TokenTypes::KEYWORD, 17),
        Token("=", TokenTypes::OPERATOR, 17),
        Token("true", TokenTypes::VALUE, 17),
        Token(";", TokenTypes::PUNCTUATION, 17),

        // Line 20
        Token("if", TokenTypes::KEYWORD, 20),
        Token("(", TokenTypes::PUNCTUATION, 20),
        Token("low", TokenTypes::IDENTIFIER, 20),
        Token("==", TokenTypes::OPERATOR, 20),
        Token("0", TokenTypes::VALUE, 20),
        Token("or", TokenTypes::KEYWORD, 20),
        Token("low", TokenTypes::IDENTIFIER, 20),
        Token("==", TokenTypes::OPERATOR, 20),
        Token("1", TokenTypes::VALUE, 20),
        Token(")", TokenTypes::PUNCTUATION, 20),

        // Line 21
        Token("{", TokenTypes::PUNCTUATION, 21),

        // Line 22
        Token("is_prime", TokenTypes::IDENTIFIER, 22),
        Token("=", TokenTypes::OPERATOR, 22),
        Token("false", TokenTypes::VALUE, 22),
        Token(";", TokenTypes::PUNCTUATION, 22),

        // Line 23
        Token("}", TokenTypes::PUNCTUATION, 23),

        // Line 25
        Token("for", TokenTypes::KEYWORD, 25),
        Token("(", TokenTypes::PUNCTUATION, 25),
        Token("let", TokenTypes::KEYWORD, 25),
        Token("i", TokenTypes::IDENTIFIER, 25),
        Token(":", TokenTypes::PUNCTUATION, 25),
        Token("int", TokenTypes::KEYWORD, 25),
        Token("=", TokenTypes::OPERATOR, 25),
        Token("2", TokenTypes::VALUE, 25),
        Token(";", TokenTypes::PUNCTUATION, 25),
        Token("i", TokenTypes::IDENTIFIER, 25),
        Token("<", TokenTypes::OPERATOR, 25),
        Token("low", TokenTypes::IDENTIFIER, 25),
        Token("//", TokenTypes::OPERATOR, 25),
        Token("2", TokenTypes::VALUE, 25),
        Token(";", TokenTypes::PUNCTUATION, 25),
        Token("i", TokenTypes::IDENTIFIER, 25),
        Token("=", TokenTypes::OPERATOR, 25),
        Token("i", TokenTypes::IDENTIFIER, 25),
        Token("+", TokenTypes::OPERATOR, 25),
        Token("1", TokenTypes::VALUE, 25),
        Token(")", TokenTypes::PUNCTUATION, 25),
        
        // Line 26
        Token("{", TokenTypes::PUNCTUATION, 26),

        // Line 27
        Token("if", TokenTypes::KEYWORD, 27),
        Token("(", TokenTypes::PUNCTUATION, 27),
        Token("low", TokenTypes::IDENTIFIER, 27),
        Token("%", TokenTypes::OPERATOR, 27),
        Token("i", TokenTypes::IDENTIFIER, 27),
        Token("==", TokenTypes::OPERATOR, 27),
        Token("0", TokenTypes::VALUE, 27),
        Token(")", TokenTypes::PUNCTUATION, 27),

        // Line 28
        Token("{", TokenTypes::PUNCTUATION, 28),

        // Line 29
        Token("is_prime", TokenTypes::IDENTIFIER, 29),
        Token("=", TokenTypes::OPERATOR, 29),
        Token("false", TokenTypes::VALUE, 29),
        Token(";", TokenTypes::PUNCTUATION, 29),
        
        // Line 30
        Token("break", TokenTypes::KEYWORD, 30),
        Token(";", TokenTypes::PUNCTUATION, 30),

        // Line 31
        Token("}", TokenTypes::PUNCTUATION, 31),

        // Line 32
        Token("}", TokenTypes::PUNCTUATION, 32),

        // Line 34
        Token("if", TokenTypes::KEYWORD, 34),
        Token("(", TokenTypes::PUNCTUATION, 34),
        Token("is_prime", TokenTypes::IDENTIFIER, 34),
        Token(")", TokenTypes::PUNCTUATION, 34),

        // Line 35  
        Token("{", TokenTypes::PUNCTUATION, 35),

        // Line 36
        Token("println", TokenTypes::IDENTIFIER, 36),
        Token("(", TokenTypes::PUNCTUATION, 36),
        Token("low", TokenTypes::IDENTIFIER, 36),
        Token(")", TokenTypes::PUNCTUATION, 36),
        Token(";", TokenTypes::PUNCTUATION, 36),

        // Line 37
        Token("}", TokenTypes::PUNCTUATION, 37),

        // Line 39
        Token("low", TokenTypes::IDENTIFIER, 39),
        Token("=", TokenTypes::OPERATOR, 39),
        Token("low", TokenTypes::IDENTIFIER, 39),
        Token("+", TokenTypes::OPERATOR, 39),
        Token("1", TokenTypes::VALUE, 39),
        Token(";", TokenTypes::PUNCTUATION, 39),

        // Line 40
        Token("}", TokenTypes::PUNCTUATION, 40),

        // Line 41
        Token("println", TokenTypes::IDENTIFIER, 41),
        Token("(", TokenTypes::PUNCTUATION, 41),
        Token(")", TokenTypes::PUNCTUATION, 41),
        Token(";", TokenTypes::PUNCTUATION, 41),

        // Line 42
        Token("}", TokenTypes::PUNCTUATION, 42),
    };

    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}
