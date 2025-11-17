#include "../../includes/utils/tests_utils.h"

#include <gtest/gtest.h>

namespace TestsUtils
{
    std::string ReadFile(const std::string& path)
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::vector<Token> CollectTokens(Lexer &lexer)
    {
        std::vector<Token> tokens;

        while (!lexer.IsFinishedReadingSRC())
        {
            Token token = lexer.Tokenize();

            if (token.GetType() != TokenTypes::EOF_TOKEN)
            {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    void CompareTokens(const std::vector<Token>& results, const std::vector<Token>& expected)
    {
        ASSERT_EQ(results.size(), expected.size()) << "Result and expected token counts differ.";

        for (size_t i = 0; i < results.size(); i++) 
        {
            EXPECT_EQ(results[i], expected[i]) << "Mismatch at token index " << i;
        }
    }
}
