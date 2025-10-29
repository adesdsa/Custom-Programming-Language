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
        EntityLogger el1("lexer");
        EntityLogger el2("parser");
        el1.Info("This is info msg");
        el2.Error("This is error msg");   
        el1.Info("This is info msg");
        el2.Warnning("This is warnning msg");
        el1.Error("This is error msg"); 
        el2.Warnning("This is warnning msg");
        //Lexer lexer;
        //lexer.Tokenize();
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