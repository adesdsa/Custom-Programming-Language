#include "includes/utils/tests_utils.h"

#include <gtest/gtest.h>  // gest FrameWork
// #region agent log
#include <fstream>
// #endregion

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

            if (token.GetType() != TokenTypes::EOF_TOKEN)
            {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    void CompareTokens(const std::vector<Token>& results, const std::vector<Token>& expected)
    {
        // #region agent log
        {
            std::ofstream logFile("c:\\Users\\Cyber_User\\Magshimim\\Final Project\\Custom-Programming-Language\\.cursor\\debug.log", std::ios::app);
            logFile << "{\"hypothesisId\":\"A\",\"location\":\"tests_utils.cpp:CompareTokens\",\"message\":\"Token counts\",\"data\":{\"results_size\":" << results.size() << ",\"expected_size\":" << expected.size() << "}}\n";
            
            // Log all result tokens to find the 2 extra
            for (size_t i = 0; i < results.size(); i++) {
                logFile << "{\"hypothesisId\":\"A\",\"location\":\"tests_utils.cpp:CompareTokens\",\"message\":\"Result token\",\"data\":{\"index\":" << i << ",\"value\":\"" << results[i].GetValue() << "\",\"type\":\"" << results[i].GetType() << "\",\"line\":" << results[i].GetLine() << ",\"col\":" << results[i].GetColumn() << "}}\n";
            }
            
            // Log expected tokens for lines 5 and 6 specifically (hypothesis A)
            logFile << "{\"hypothesisId\":\"A\",\"location\":\"tests_utils.cpp:CompareTokens\",\"message\":\"Expected tokens count for comparison\",\"data\":{\"expected_size\":" << expected.size() << "}}\n";
            logFile.close();
        }
        // #endregion

        ASSERT_EQ(results.size(), expected.size()) << "Result and expected token counts differ.";

        for (size_t i = 0; i < results.size(); i++) 
        {
            EXPECT_EQ(results[i], expected[i]) << "Mismatch at token index " << i;
        }
    }
} 
