#include <iostream>

#include "includes/tokenizer/lexer.h"
#include "includes/logger/entity_logger.h"

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
        
        while(!lexer.IsFinishedReadingSRC()) 
        {
            auto token = lexer.Tokenize();

            if(token.has_value())
            {
                std::cout << token.value().GetType() << " - " << token.value().GetValue() << " - " << token.value().GetLine() << std::endl;
            }
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
