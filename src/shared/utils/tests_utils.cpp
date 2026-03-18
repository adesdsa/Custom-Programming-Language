#include "includes/shared/utils/tests_utils.h"

#include <algorithm>
#include <gtest/gtest.h>  // gtest FrameWork

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
            auto optToken = lexer.Tokenize();

            if(!optToken) continue;

            Token token = std::move(*optToken);

            if (token.GetType() != TokenTypes::EOF_TOKEN &&
                token.GetType() != TokenTypes::TRIVIA &&
                token.GetType() != TokenTypes::UNFINISHED_TRIVIA)
            {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    std::vector<Token> FilterTrivia(std::vector<Token> tokens)
    {
        tokens.erase(
            std::remove_if(tokens.begin(), tokens.end(),
                [](const Token& t) {
                    return t.GetType() == TokenTypes::TRIVIA ||
                           t.GetType() == TokenTypes::UNFINISHED_TRIVIA;
                }),
            tokens.end());
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
