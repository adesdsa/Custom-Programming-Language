#include "../../includes/utils/tests_utils.h"

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

    std::vector<Token> FileUtils::CollectTokens(Lexer &lexer)
    {
        std::vector<Token> tokens;

        while (!lexer.IsFinishedReadingSRC())
        {
            auto token = lexer.Tokenize();

            if (token.has_value())
            { 
                tokens.push_back(token.value());
            }
        }
        return tokens;
    }

    void CompareTokens(const std::vector<Token>& results, const std::vector<Token>& expected)
    {
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
}