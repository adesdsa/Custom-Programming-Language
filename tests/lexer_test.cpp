#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/tokenizer/lexer.h"
#include "includes/utils/tests_utils.h"
#include "includes/utils/system_utils.h"

// TEST: Empty source file
TEST(LexerTest, EmptyInput) 
{
    std::vector<Token> expected;

    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/test1.picoc"));
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}

// TEST: Only trivia tokens in source file
TEST(LexerTest, OnlyTriviaTokens) 
{
    std::vector<Token> expected;

    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/test2.picoc"));
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);
    
    TestsUtils::CompareTokens(results, expected);
}

// TEST: A small code example
TEST(LexerTest, SmallCodeExample) 
{
    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/test3.picoc"));

    std::vector<Token> expected = {
        // Line 5
        Token("Add", TokenTypes::IDENTIFIER,5, 1),
        Token("(", TokenTypes::PUNCTUATION,5, 4),
        Token("let",TokenTypes::KEYWORD,5, 5),
        Token("a",TokenTypes::IDENTIFIER,5, 9),
        Token(":",TokenTypes::PUNCTUATION,5, 10),
        Token("int",TokenTypes::KEYWORD,5, 12),
        Token(",",TokenTypes::PUNCTUATION,5, 15),
        Token("let",TokenTypes::KEYWORD,5, 17),
        Token("b",TokenTypes::IDENTIFIER,5, 21),
        Token(":",TokenTypes::PUNCTUATION,5, 22),
        Token("int",TokenTypes::KEYWORD,5, 24),
        Token(")",TokenTypes::PUNCTUATION,5, 27),
        Token("->",TokenTypes::PUNCTUATION,5, 29),
        Token("void",TokenTypes::KEYWORD,5, 32),

        // Line 6
        Token("{",TokenTypes::PUNCTUATION,6, 1),

        // Line 7
        Token("println",TokenTypes::IDENTIFIER,7, 5),
        Token("(", TokenTypes::PUNCTUATION,7, 12),
        Token("3", TokenTypes::VALUE,7, 13),
        Token("+", TokenTypes::OPERATOR,7, 15),
        Token("2", TokenTypes::VALUE,7, 17),
        Token("==", TokenTypes::OPERATOR,7, 19),
        Token("5", TokenTypes::VALUE,7, 22),
        Token(")",TokenTypes::PUNCTUATION,7, 23),
        Token(";",TokenTypes::PUNCTUATION,7, 24),

        // Line 8
        Token("}",TokenTypes::PUNCTUATION,8, 1),
    };

    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}

// TEST: Full program example
TEST(LexerTest, FullProgram) 
{
    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/test4.picoc"));

    std::vector<Token> expected = {
        // Line 1
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 1, 1),
        Token("(", TokenTypes::PUNCTUATION,1,30),
        Token("low", TokenTypes::IDENTIFIER,1,31),
        Token(":", TokenTypes::PUNCTUATION,1,34),
        Token("int", TokenTypes::KEYWORD,1,36),
        Token(",", TokenTypes::PUNCTUATION,1,39),
        Token("high", TokenTypes::IDENTIFIER,1,41),
        Token(":", TokenTypes::PUNCTUATION,1,45),
        Token("int", TokenTypes::KEYWORD,1,47),
        Token(")", TokenTypes::PUNCTUATION,1,50),
        Token("->", TokenTypes::PUNCTUATION,1,52),
        Token("void", TokenTypes::KEYWORD,1,55),
        Token(";", TokenTypes::PUNCTUATION, 1, 59),

        // Line 3
        Token("main", TokenTypes::IDENTIFIER, 3, 1),
        Token("(", TokenTypes::PUNCTUATION, 3, 5),
        Token("void", TokenTypes::KEYWORD, 3, 6),
        Token(")", TokenTypes::PUNCTUATION, 3, 10),
        Token("->", TokenTypes::PUNCTUATION, 3, 12),
        Token("int", TokenTypes::KEYWORD, 3, 15),

        // Line 4
        Token("{", TokenTypes::PUNCTUATION, 4, 1),

        // Line 5
        Token("let", TokenTypes::KEYWORD, 5, 5),
        Token("low", TokenTypes::IDENTIFIER, 5, 9),
        Token(":", TokenTypes::PUNCTUATION, 5, 12),
        Token("int", TokenTypes::KEYWORD, 5, 14),
        Token("=", TokenTypes::OPERATOR, 5, 18),
        Token("1", TokenTypes::VALUE, 5, 20),    
        Token(";", TokenTypes::PUNCTUATION, 5, 21),

        // Line 6
        Token("let", TokenTypes::KEYWORD, 6, 5),
        Token("high", TokenTypes::IDENTIFIER, 6, 9),
        Token(":", TokenTypes::PUNCTUATION, 6, 13),
        Token("int", TokenTypes::KEYWORD, 6, 15),
        Token("=", TokenTypes::OPERATOR, 6, 19),
        Token("100", TokenTypes::VALUE, 6, 21),
        Token(";", TokenTypes::PUNCTUATION, 6, 24),

        // Line 8
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 8, 5),
        Token("(", TokenTypes::PUNCTUATION, 8, 34),
        Token("low", TokenTypes::IDENTIFIER, 8, 35),
        Token(",", TokenTypes::PUNCTUATION, 8, 38),
        Token("high", TokenTypes::IDENTIFIER, 8, 40),
        Token(")", TokenTypes::PUNCTUATION, 8, 44),
        Token(";", TokenTypes::PUNCTUATION, 8, 45),

        // Line 9
        Token("return", TokenTypes::KEYWORD, 9, 5),
        Token("0", TokenTypes::VALUE, 9, 12),
        Token(";", TokenTypes::PUNCTUATION, 9, 13),

        // Line 10
        Token("}", TokenTypes::PUNCTUATION, 10, 1),

        // Line 13
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 13, 1),
        Token("(", TokenTypes::PUNCTUATION, 13, 30),
        Token("low", TokenTypes::IDENTIFIER, 13, 31),
        Token(":", TokenTypes::PUNCTUATION, 13, 34),
        Token("int", TokenTypes::KEYWORD, 13, 36),
        Token(",", TokenTypes::PUNCTUATION, 13, 39),
        Token("high", TokenTypes::IDENTIFIER, 13, 41),
        Token(":", TokenTypes::PUNCTUATION, 13, 45),
        Token("int", TokenTypes::KEYWORD, 13, 47),
        Token(")", TokenTypes::PUNCTUATION, 13, 50),
        Token("->", TokenTypes::PUNCTUATION, 13, 52),
        Token("void", TokenTypes::KEYWORD, 13, 55),

        // Line 14
        Token("{", TokenTypes::PUNCTUATION, 14, 1),

        // Line 15
        Token("while", TokenTypes::KEYWORD, 15, 5),
        Token("(", TokenTypes::PUNCTUATION, 15, 10),
        Token("low", TokenTypes::IDENTIFIER, 15, 11),
        Token("<", TokenTypes::OPERATOR, 15, 15),
        Token("high", TokenTypes::IDENTIFIER, 15, 17),
        Token(")", TokenTypes::PUNCTUATION, 15, 21),

        // Line 16
        Token("{", TokenTypes::PUNCTUATION, 16, 5),

        // Line 17
        Token("let", TokenTypes::KEYWORD, 17, 9),
        Token("is_prime", TokenTypes::IDENTIFIER, 17, 13),
        Token(":", TokenTypes::PUNCTUATION, 17, 21),
        Token("bool", TokenTypes::KEYWORD, 17, 23),
        Token("=", TokenTypes::OPERATOR, 17, 28),
        Token("true", TokenTypes::VALUE, 17, 30),
        Token(";", TokenTypes::PUNCTUATION, 17, 34),

        // Line 20
        Token("if", TokenTypes::KEYWORD, 20, 9),
        Token("(", TokenTypes::PUNCTUATION, 20, 11),
        Token("low", TokenTypes::IDENTIFIER, 20, 12),
        Token("==", TokenTypes::OPERATOR, 20, 16),
        Token("0", TokenTypes::VALUE, 20, 19),
        Token("or", TokenTypes::KEYWORD, 20, 21),
        Token("low", TokenTypes::IDENTIFIER, 20, 24),
        Token("==", TokenTypes::OPERATOR, 20, 28),
        Token("1", TokenTypes::VALUE, 20, 31),
        Token(")", TokenTypes::PUNCTUATION, 20, 32),

        // Line 21
        Token("{", TokenTypes::PUNCTUATION, 21, 9),

        // Line 22
        Token("is_prime", TokenTypes::IDENTIFIER, 22, 13),
        Token("=", TokenTypes::OPERATOR, 22, 22),
        Token("false", TokenTypes::VALUE, 22, 24),
        Token(";", TokenTypes::PUNCTUATION, 22, 29),

        // Line 23
        Token("}", TokenTypes::PUNCTUATION, 23, 9),

        // Line 25
        Token("for", TokenTypes::KEYWORD, 25, 9),
        Token("(", TokenTypes::PUNCTUATION, 25, 12),
        Token("let", TokenTypes::KEYWORD, 25, 13),
        Token("i", TokenTypes::IDENTIFIER, 25, 17),
        Token(":", TokenTypes::PUNCTUATION, 25, 18),
        Token("int", TokenTypes::KEYWORD, 25, 20),
        Token("=", TokenTypes::OPERATOR, 25, 24),
        Token("2", TokenTypes::VALUE, 25, 26),
        Token(";", TokenTypes::PUNCTUATION, 25, 27),
        Token("i", TokenTypes::IDENTIFIER, 25, 29),
        Token("<", TokenTypes::OPERATOR, 25, 31),
        Token("low", TokenTypes::IDENTIFIER, 25, 33),
        Token("//", TokenTypes::OPERATOR, 25, 37),
        Token("2", TokenTypes::VALUE, 25, 40),
        Token(";", TokenTypes::PUNCTUATION, 25, 41),
        Token("i", TokenTypes::IDENTIFIER, 25, 43),
        Token("=", TokenTypes::OPERATOR, 25, 45),
        Token("i", TokenTypes::IDENTIFIER, 25, 47),
        Token("+", TokenTypes::OPERATOR, 25, 49),
        Token("1", TokenTypes::VALUE, 25, 51),
        Token(")", TokenTypes::PUNCTUATION, 25, 52),
        
        // Line 26
        Token("{", TokenTypes::PUNCTUATION, 26, 9),

        // Line 27
        Token("if", TokenTypes::KEYWORD, 27, 13),
        Token("(", TokenTypes::PUNCTUATION, 27, 15),
        Token("low", TokenTypes::IDENTIFIER, 27, 16),
        Token("%", TokenTypes::OPERATOR, 27, 20),
        Token("i", TokenTypes::IDENTIFIER, 27, 22),
        Token("==", TokenTypes::OPERATOR, 27, 24),
        Token("0", TokenTypes::VALUE, 27, 27),
        Token(")", TokenTypes::PUNCTUATION, 27, 28),

        // Line 28
        Token("{", TokenTypes::PUNCTUATION, 28, 13),

        // Line 29
        Token("is_prime", TokenTypes::IDENTIFIER, 29, 17),
        Token("=", TokenTypes::OPERATOR, 29, 26),
        Token("false", TokenTypes::VALUE, 29, 28),
        Token(";", TokenTypes::PUNCTUATION, 29, 33),
        
        // Line 30
        Token("break", TokenTypes::KEYWORD, 30, 17),
        Token(";", TokenTypes::PUNCTUATION, 30, 22),

        // Line 31
        Token("}", TokenTypes::PUNCTUATION, 31, 13),

        // Line 32
        Token("}", TokenTypes::PUNCTUATION, 32, 9),

        // Line 34
        Token("if", TokenTypes::KEYWORD, 34, 9),
        Token("(", TokenTypes::PUNCTUATION, 34, 11),
        Token("is_prime", TokenTypes::IDENTIFIER, 34, 12),
        Token(")", TokenTypes::PUNCTUATION, 34, 20),

        // Line 35  
        Token("{", TokenTypes::PUNCTUATION, 35, 9),

        /**
        // Line 36
        Token("println", TokenTypes::IDENTIFIER, 36, 13),
        Token("(", TokenTypes::PUNCTUATION, 36, 20),
        Token("low", TokenTypes::IDENTIFIER, 36, 21),
        Token(")", TokenTypes::PUNCTUATION, 36, 24),
        Token(";", TokenTypes::PUNCTUATION, 36, 25),
        */

        // Line 37
        Token("}", TokenTypes::PUNCTUATION, 37, 9),

        // Line 39
        Token("low", TokenTypes::IDENTIFIER, 39, 9),
        Token("=", TokenTypes::OPERATOR, 39, 13),
        Token("low", TokenTypes::IDENTIFIER, 39, 15),
        Token("+", TokenTypes::OPERATOR, 39, 19),
        Token("1", TokenTypes::VALUE, 39, 21),
        Token(";", TokenTypes::PUNCTUATION, 39, 22),

        // Line 40
        Token("}", TokenTypes::PUNCTUATION, 40, 5),

        /**
        // Line 41
        Token("println", TokenTypes::IDENTIFIER, 41, 5),
        Token("(", TokenTypes::PUNCTUATION, 41, 12),
        Token(")", TokenTypes::PUNCTUATION, 41, 13),
        Token(";", TokenTypes::PUNCTUATION, 41, 14),
        */

        // Line 42
        Token("}", TokenTypes::PUNCTUATION, 42, 1),
    };

    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}
