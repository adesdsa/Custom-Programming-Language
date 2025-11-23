#include <iostream>

#include "includes/utils/tests_utils.h"

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
        Lexer lexer("../../tests/inputs/test4.txt");
        
        auto tokens = TestsUtils::CollectTokens(lexer);

        for(const auto token : tokens)
        {
            std::cout << "Line: " << token.GetLine() << 
            " - Type: " << token.GetType() <<
            " - Value: " << token.GetValue() << std::endl;
        }
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