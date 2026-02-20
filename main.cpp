#include <iostream>

#include "includes/compiler_orchestrator.h"
#include "includes/driver/ui/cli/cli_parser.h"

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
        CompilerOptions options = CLIParser::Parse(argc, argv);

        // Handle standalone CLI commands before launching the pipeline
        if (options.displayHelp)
        {
            CLIParser::PrintUsage();
            return ReturnCode::k_nSuccess;
        }

        if (options.displayVersion)
        {
            CLIParser::PrintVersion();
            return ReturnCode::k_nSuccess;
        }

        CompilerOrchestrator compilerOrchestrator(std::move(options));
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
