#include <iostream>
#include <fstream> //

#include "includes/tokenizer/lexer.h"
#include "includes/tokenizer/token.h" //
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
        Lexer lexer;
        std::ofstream file("output.txt");
        
        //while(lexer.Tokenize() != std::nullopt);
        for(int i = 0; i < 36; i++)
        {
            auto tokenOpt = lexer.Tokenize();
            if(tokenOpt != std::nullopt)
            {
                std::cout << 1 << std::endl;
                file << tokenOpt->GetType() << "(" << tokenOpt->GetValue() << ")" << tokenOpt->GetLine() << " ";
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