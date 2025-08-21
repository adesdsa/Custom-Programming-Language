#include <gtest/gtest.h>

#include <filesystem>

#include "tokenizer/lexer.h"





// TEST: Empty source file.
// INPUT: Empty source file.
// EXPECTED: No tokens created.
TEST(LexerTest, EmptyInput) 
{
    std::ofstream streamSrcFile("../tests/inputs/test1.txt");
    ASSERT_TRUE(streamSrcFile.is_open());

    streamSrcFile.close();

    Lexer lexer;
    lexer.Tokenize();

    std::ifstream streamOutputFile("../tests/expected/test1.txt");
    ASSERT_TRUE(streamOutputFile.is_open());

    std::string tokens;
    std::getline(streamOutputFile, tokens);
    streamOutputFile.close();

    EXPECT_EQ(tokens.empty());
}

// TEST: Only trivia tokens in source file.
// INPUT: Source file with different trivia tokens.
// EXPECTED: No tokens created.
TEST(LexerTest, OnlyTriviaTokens) 
{
    std::ofstream streamSrcFile(k_sInputFile);
    ASSERT_TRUE(streamSrcFile.is_open());
    
    streamSrcFile << ;
    streamSrcFile.close();

    Lexer lexer;
    lexer.Tokenize();

    std::ifstream streamOutputFile(k_sOutputFile);
    ASSERT_TRUE(streamOutputFile.is_open());

    std::string tokens;
    std::getline(streamOutputFile, tokens);
    streamOutputFile.close();

    EXPECT_TRUE(tokens.empty()); 
}