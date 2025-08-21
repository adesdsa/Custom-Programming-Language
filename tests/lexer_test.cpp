#include <gtest/gtest.h>

#include "tokenizer/lexer.h"

// TEST: Empty source file.
// INPUT: Empty source file.
// EXPECTED: No tokens created.
TEST(LexerTest, EmptyInput) 
{
    std::ofstream streamSrcFile(k_sInputFile);
    ASSERT_TRUE(streamSrcFile.is_open());

    streamSrcFile.clear();
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