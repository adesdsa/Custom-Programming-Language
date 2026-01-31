#include <iostream>

#include "includes/compiler_orchestrator.h"

namespace ReturnCode
{
    inline constexpr const int k_nSuccess = 0;
    inline constexpr const int k_nFailed = 1;
}

int main(int argc, const char* argv[])
{
    int nReturnValue = ReturnCode::k_nSuccess;

    try
    {
        int fakeArgc = 2;
        const char* fakeArgv[] = { "", "" };
        CompilerOrchestrator compilerOrchestrator(fakeArgc, fakeArgv);
        compilerOrchestrator.MainFlow();
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
