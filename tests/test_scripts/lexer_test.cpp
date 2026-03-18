#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/frontend/tokenizer/lexer.h"
#include "includes/shared/utils/tests_utils.h"
#include "includes/shared/utils/system_utils.h"

// TEST: Empty source file
TEST(LexerTest, EmptyInput) 
{
    std::vector<Token> expected;

    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/empty_file.picoc"));
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}

// TEST: Only trivia tokens in source file
TEST(LexerTest, OnlyTriviaTokens) 
{
    std::vector<Token> expected;

    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/only_trivia_tokens_file.picoc"));
    std::vector<Token> results = TestsUtils::CollectTokens(lexer);
    
    TestsUtils::CompareTokens(results, expected);
}

// TEST: A small code example
TEST(LexerTest, SmallCodeExample) 
{
    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/small_code_example_file.picoc"));

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
        Token("int",TokenTypes::KEYWORD,5, 32),

        // Line 6
        Token("{",TokenTypes::PUNCTUATION,6, 1),

        // Line 7
        Token("return",TokenTypes::KEYWORD,7, 5),
        Token("a", TokenTypes::IDENTIFIER,7, 12),
        Token("+", TokenTypes::OPERATOR,7, 14),
        Token("b", TokenTypes::IDENTIFIER,7, 16),
        Token(";",TokenTypes::PUNCTUATION,7, 17),

        // Line 8
        Token("}",TokenTypes::PUNCTUATION,8, 1),
    };

    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}

// TEST: Full program example
TEST(LexerTest, FullProgram) 
{
    Lexer lexer(SystemUtils::BuildRelativePath(L"tests/inputs/full_program_example_file.picoc"));

    std::vector<Token> expected = {
        // Line 1: import {println} from "io"
        Token("import", TokenTypes::KEYWORD, 1, 1),
        Token("{", TokenTypes::PUNCTUATION, 1, 8),
        Token("println", TokenTypes::IDENTIFIER, 1, 9),
        Token("}", TokenTypes::PUNCTUATION, 1, 16),
        Token("from", TokenTypes::KEYWORD, 1, 18),
        Token("\"io\"", TokenTypes::STRING_LITERAL, 1, 23),

        // Line 3
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 3, 1),
        Token("(", TokenTypes::PUNCTUATION, 3, 30),
        Token("low", TokenTypes::IDENTIFIER, 3, 31),
        Token(":", TokenTypes::PUNCTUATION, 3, 34),
        Token("int", TokenTypes::KEYWORD, 3, 36),
        Token(",", TokenTypes::PUNCTUATION, 3, 39),
        Token("high", TokenTypes::IDENTIFIER, 3, 41),
        Token(":", TokenTypes::PUNCTUATION, 3, 45),
        Token("int", TokenTypes::KEYWORD, 3, 47),
        Token(")", TokenTypes::PUNCTUATION, 3, 50),
        Token("->", TokenTypes::PUNCTUATION, 3, 52),
        Token("void", TokenTypes::KEYWORD, 3, 55),
        Token(";", TokenTypes::PUNCTUATION, 3, 59),

        // Line 5
        Token("main", TokenTypes::IDENTIFIER, 5, 1),
        Token("(", TokenTypes::PUNCTUATION, 5, 5),
        Token("void", TokenTypes::KEYWORD, 5, 6),
        Token(")", TokenTypes::PUNCTUATION, 5, 10),
        Token("->", TokenTypes::PUNCTUATION, 5, 12),
        Token("int", TokenTypes::KEYWORD, 5, 15),

        // Line 6
        Token("{", TokenTypes::PUNCTUATION, 6, 1),

        // Line 7
        Token("let", TokenTypes::KEYWORD, 7, 5),
        Token("low", TokenTypes::IDENTIFIER, 7, 9),
        Token(":", TokenTypes::PUNCTUATION, 7, 12),
        Token("int", TokenTypes::KEYWORD, 7, 14),
        Token("=", TokenTypes::OPERATOR, 7, 18),
        Token("1", TokenTypes::VALUE, 7, 20),    
        Token(";", TokenTypes::PUNCTUATION, 7, 21),

        // Line 8
        Token("let", TokenTypes::KEYWORD, 8, 5),
        Token("high", TokenTypes::IDENTIFIER, 8, 9),
        Token(":", TokenTypes::PUNCTUATION, 8, 13),
        Token("int", TokenTypes::KEYWORD, 8, 15),
        Token("=", TokenTypes::OPERATOR, 8, 19),
        Token("100", TokenTypes::VALUE, 8, 21),
        Token(";", TokenTypes::PUNCTUATION, 8, 24),


        // Line 10
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 10, 5),
        Token("(", TokenTypes::PUNCTUATION, 10, 34),
        Token("low", TokenTypes::IDENTIFIER, 10, 35),
        Token(",", TokenTypes::PUNCTUATION, 10, 38),
        Token("high", TokenTypes::IDENTIFIER, 10, 40),
        Token(")", TokenTypes::PUNCTUATION, 10, 44),
        Token(";", TokenTypes::PUNCTUATION, 10, 45),

        // Line 11
        Token("return", TokenTypes::KEYWORD, 11, 5),
        Token("0", TokenTypes::VALUE, 11, 12),
        Token(";", TokenTypes::PUNCTUATION, 11, 13),

        // Line 12
        Token("}", TokenTypes::PUNCTUATION, 12, 1),

        // Line 15
        Token("displayAllPrimeNumbersInRange", TokenTypes::IDENTIFIER, 15, 1),
        Token("(", TokenTypes::PUNCTUATION, 15, 30),
        Token("low", TokenTypes::IDENTIFIER, 15, 31),
        Token(":", TokenTypes::PUNCTUATION, 15, 34),
        Token("int", TokenTypes::KEYWORD, 15, 36),
        Token(",", TokenTypes::PUNCTUATION, 15, 39),
        Token("high", TokenTypes::IDENTIFIER, 15, 41),
        Token(":", TokenTypes::PUNCTUATION, 15, 45),
        Token("int", TokenTypes::KEYWORD, 15, 47),
        Token(")", TokenTypes::PUNCTUATION, 15, 50),
        Token("->", TokenTypes::PUNCTUATION, 15, 52),
        Token("void", TokenTypes::KEYWORD, 15, 55),

        // Line 16
        Token("{", TokenTypes::PUNCTUATION, 16, 1),

        // Line 17
        Token("while", TokenTypes::KEYWORD, 17, 5),
        Token("(", TokenTypes::PUNCTUATION, 17, 10),
        Token("low", TokenTypes::IDENTIFIER, 17, 11),
        Token("<", TokenTypes::OPERATOR, 17, 15),
        Token("high", TokenTypes::IDENTIFIER, 17, 17),
        Token(")", TokenTypes::PUNCTUATION, 17, 21),

        // Line 18
        Token("{", TokenTypes::PUNCTUATION, 18, 5),

        // Line 19
        Token("let", TokenTypes::KEYWORD, 19, 9),
        Token("is_prime", TokenTypes::IDENTIFIER, 19, 13),
        Token(":", TokenTypes::PUNCTUATION, 19, 21),
        Token("bool", TokenTypes::KEYWORD, 19, 23),
        Token("=", TokenTypes::OPERATOR, 19, 28),
        Token("true", TokenTypes::VALUE, 19, 30),
        Token(";", TokenTypes::PUNCTUATION, 19, 34),

        // Line 22
        Token("if", TokenTypes::KEYWORD, 22, 9),
        Token("(", TokenTypes::PUNCTUATION, 22, 11),
        Token("low", TokenTypes::IDENTIFIER, 22, 12),
        Token("<", TokenTypes::OPERATOR, 22, 16),
        Token("2", TokenTypes::VALUE, 22, 18),
        Token(")", TokenTypes::PUNCTUATION, 22, 19),

        // Line 23
        Token("{", TokenTypes::PUNCTUATION, 23, 9),

        // Line 24
        Token("is_prime", TokenTypes::IDENTIFIER, 24, 13),
        Token("=", TokenTypes::OPERATOR, 24, 22),
        Token("false", TokenTypes::VALUE, 24, 24),
        Token(";", TokenTypes::PUNCTUATION, 24, 29),

        // Line 25
        Token("}", TokenTypes::PUNCTUATION, 25, 9),

        // Line 27
        Token("for", TokenTypes::KEYWORD, 27, 9),
        Token("(", TokenTypes::PUNCTUATION, 27, 12),
        Token("let", TokenTypes::KEYWORD, 27, 13),
        Token("i", TokenTypes::IDENTIFIER, 27, 17),
        Token(":", TokenTypes::PUNCTUATION, 27, 18),
        Token("int", TokenTypes::KEYWORD, 27, 20),
        Token("=", TokenTypes::OPERATOR, 27, 24),
        Token("2", TokenTypes::VALUE, 27, 26),
        Token(";", TokenTypes::PUNCTUATION, 27, 27),
        Token("i", TokenTypes::IDENTIFIER, 27, 29),
        Token("<", TokenTypes::OPERATOR, 27, 31),
        Token("(", TokenTypes::PUNCTUATION, 27, 33),
        Token("(", TokenTypes::PUNCTUATION, 27, 34),
        Token("low", TokenTypes::IDENTIFIER, 27, 35),
        Token("//", TokenTypes::OPERATOR, 27, 39),
        Token("2", TokenTypes::VALUE, 27, 42),
        Token(")", TokenTypes::PUNCTUATION, 27, 43),
        Token("+", TokenTypes::OPERATOR, 27, 45),
        Token("1", TokenTypes::VALUE, 27, 47),
        Token(")", TokenTypes::PUNCTUATION, 27, 48),
        Token(";", TokenTypes::PUNCTUATION, 27, 50),
        Token("i", TokenTypes::IDENTIFIER, 27, 52),
        Token("=", TokenTypes::OPERATOR, 27, 54),
        Token("i", TokenTypes::IDENTIFIER, 27, 56),
        Token("+", TokenTypes::OPERATOR, 27, 58),
        Token("1", TokenTypes::VALUE, 27, 60),
        Token(")", TokenTypes::PUNCTUATION, 27, 61),
        
        // Line 28
        Token("{", TokenTypes::PUNCTUATION, 28, 9),

        // Line 29
        Token("if", TokenTypes::KEYWORD, 29, 13),
        Token("(", TokenTypes::PUNCTUATION, 29, 15),
        Token("low", TokenTypes::IDENTIFIER, 29, 16),
        Token("%", TokenTypes::OPERATOR, 29, 20),
        Token("i", TokenTypes::IDENTIFIER, 29, 22),
        Token("==", TokenTypes::OPERATOR, 29, 24),
        Token("0", TokenTypes::VALUE, 29, 27),
        Token(")", TokenTypes::PUNCTUATION, 29, 28),

        // Line 30
        Token("{", TokenTypes::PUNCTUATION, 30, 13),

        // Line 31
        Token("is_prime", TokenTypes::IDENTIFIER, 31, 17),
        Token("=", TokenTypes::OPERATOR, 31, 26),
        Token("false", TokenTypes::VALUE, 31, 28),
        Token(";", TokenTypes::PUNCTUATION, 31, 33),
        
        // Line 32
        Token("break", TokenTypes::KEYWORD, 32, 17),
        Token(";", TokenTypes::PUNCTUATION, 32, 22),

        // Line 33
        Token("}", TokenTypes::PUNCTUATION, 33, 13),

        // Line 34
        Token("}", TokenTypes::PUNCTUATION, 34, 9),

        // Line 36
        Token("if", TokenTypes::KEYWORD, 36, 9),
        Token("(", TokenTypes::PUNCTUATION, 36, 11),
        Token("is_prime", TokenTypes::IDENTIFIER, 36, 12),
        Token(")", TokenTypes::PUNCTUATION, 36, 20),

        // Line 37  
        Token("{", TokenTypes::PUNCTUATION, 37, 9),

        // Line 38
        Token("println", TokenTypes::IDENTIFIER, 38, 13),
        Token("(", TokenTypes::PUNCTUATION, 38, 20),
        Token("low", TokenTypes::IDENTIFIER, 38, 21),
        Token(")", TokenTypes::PUNCTUATION, 38, 24),
        Token(";", TokenTypes::PUNCTUATION, 38, 25),

        // Line 39
        Token("}", TokenTypes::PUNCTUATION, 39, 9),

        // Line 41
        Token("low", TokenTypes::IDENTIFIER, 41, 9),
        Token("=", TokenTypes::OPERATOR, 41, 13),
        Token("low", TokenTypes::IDENTIFIER, 41, 15),
        Token("+", TokenTypes::OPERATOR, 41, 19),
        Token("1", TokenTypes::VALUE, 41, 21),
        Token(";", TokenTypes::PUNCTUATION, 41, 22),

        // Line 42
        Token("}", TokenTypes::PUNCTUATION, 42, 5),

        // Line 43 is a comment: # println(); Display with \n
        // (no tokens emitted for comments)

        // Line 44
        Token("}", TokenTypes::PUNCTUATION, 44, 1),
    };

    std::vector<Token> results = TestsUtils::CollectTokens(lexer);

    TestsUtils::CompareTokens(results, expected);
}
