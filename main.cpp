#include <iostream>

#include "includes/utils/tests_utils.h"
#include "includes/utils/system_utils.h"
#include "includes/parser/parser_logic.h"

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
        ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/test4.txt"));
        parser.MainFlow();
    }
    catch(const CompilationAbortException& abortError)
    {
        std::cerr << "ERROR: " << abortError.what() << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
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
