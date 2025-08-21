#include <gtest/gtest.h>

#include "tokenizer/lexer.h"


TEST(LexerTest, EmptyInput) 
{
    Lexer lexer;
    lexer.Tokenize();

    std::ifstream streamOutputFile(k_sOutputFile);
    ASSERT_TRUE(streamOutputFile.is_open());

    std::string tokens;
    std::getline(streamOutputFile, tokens);
    streamOutputFile.close();

    EXPECT_TRUE(tokens.empty());
}