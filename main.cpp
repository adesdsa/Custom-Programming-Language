#include <iostream>

#include "utils/tests_utils.h"
#include "parser_component/commandLine.h"

namespace ReturnCode
{
    inline constexpr const int k_nSuccess = 0;
    inline constexpr const int k_nFailed = 1;
}

int main(void)
{
    int nReturnValue = ReturnCode::k_nSuccess;

    try
    {
        Lexer lexer("../../../tests/inputs/test4.txt");
        
        auto tokens = TestsUtils::CollectTokens(lexer);
        Token currToken = tokens[0];
        Token nextToken = tokens[1];
        CommandLine cl(lexer.GetEntityLogger());

        for (int i = 1; i < tokens.size(); i++)
        {
            currToken = tokens[i-1];
            nextToken = tokens[i];
            cl.AddToken(currToken, nextToken);
            if (cl.IsFinished())
            {
                std::cout << "Is legal: " << cl.IsLegal() << " Line: " << cl.GetLine()[0].GetTokens()[0].GetLine();
                for (int i = 0; i < cl.GetLine().size(); i++)
                {
                    std::cout << " " << cl.GetLine()[i].GetType();
                }
                std::cout << std::endl;
                cl = CommandLine(lexer.GetEntityLogger());
            }
        }

        /*for(const auto token : tokens)
        {
            std::cout << "Line: " << token.GetLine() << 
            " - Type: " << token.GetType() <<
            " - Value: " << token.GetValue() << std::endl;
        }*/
    }

    catch(const std::runtime_error& runtimeError)
    {
        std::cerr << "Run time ERROR: " << runtimeError.what() << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
    }
    catch(const std::exception& generalError)
    {
        std::cerr << "General ERROR: " << generalError.what() << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
    }
    catch(...)
    {
        std::cerr << "Unknown ERROR" << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
    }

    return nReturnValue;
}
