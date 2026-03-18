/// *******************************************************************************
// Copyright (c) PicoC
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.
/// *******************************************************************************
#include <iostream>

#include "includes/compiler_orchestrator.h"
#include "includes/driver/ui/cli/cli_parser.h"

namespace ReturnCode
{
    inline constexpr const int k_nSuccess = 0;
    inline constexpr const int k_nCompilationAbort = 1;
    inline constexpr const int k_nRuntimeFailure = 2;
    inline constexpr const int k_nGeneralError = 3;
    inline constexpr const int k_nUknownError = -1;
}

int main(int argc, const char** argv)
{
    try
    {
        CompilerOptions options = CLIParser::Parse(argc, argv);

        // handle standalone CLI commands before launching the pipeline
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
        return ReturnCode::k_nCompilationAbort;
    }
    catch(const std::runtime_error& runtimeError)
    {
        std::cerr << "Run time ERROR: " << runtimeError.what() << std::endl;
        return ReturnCode::k_nRuntimeFailure;
    }
    catch(const std::exception& generalError)
    {
        std::cerr << "General ERROR: " << generalError.what() << std::endl;
        return ReturnCode::k_nGeneralError;
    }
    catch(...)
    {
        std::cerr << "Unknown ERROR" << std::endl;
        return ReturnCode::k_nUknownError;
    }

    return ReturnCode::k_nSuccess;
}
