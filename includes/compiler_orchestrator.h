#pragma once

#include <vector>
#include <string>
#include <exception>
#include <cstring>

#include "includes/utils/system_utils.h"
#include "includes/parser/parser_logic.h"

// Manager of all compilers components.
// performs the complete compiling process.             
// from request from UI for compilation to final form:
//
// native executable:
//  - PE (.exe) on Windows
//  - ELF on Linux
class CompilerOrchestrator
{
public:
    // constructor using Command Line Arguments
    CompilerOrchestrator(int argc, const char* argv[]);

    /*****************************************************************************
     * @brief Runs Whole Compiler's Pipeline:                                    |
     *                                                                           |
     * This method performs the complete compiling process.                      |
     * Takes input source files and compilation flags, and produces a native     |
     * executable:                                                               |
     *  - PE (.exe) on Windows                                                   |
     *  - ELF on Linux                                                           |
     *                                                                           |
     * 1. @param Request from UI'S (CLI/GUI) with project files paths & flags    |                                                                       |
     *    @brief Deserializer of request                                         |
     *    @return List of all files paths &                                      |
     *      Map - Key: Flag, Value: It's arguments list (if any)                 |
     *                                                                           |
     * 2. For each file:                                                         |
     *                                                                           |
     * 2.1. @param file_path                                                     |
     *      @brief Parse & Syntax Checker, @see includes/parser                  |                                        |
     *      @return AST                                                          |
     *                                                                           |
     * 2.2 @param AST                                                            |
     *     @brief Semantic Analysis, @see includes/semantic_analysis             |
     *     @return Decorated AST - AST + VarTable Tree + FuncTable               |                                                          |
     *                                                                           |
     * 2.3 @param Decorated AST - @see step 2.2                                  |
     *     @brief Intermediate Representation - Optimization & Define RunTime    |
     *     @return x86 .asm file                                                 |
     *                                                                           |
     * 2.4 @param x86 .asm file                                                  |
     *     @brief Assembler via NASM API                                         |
     *     @return .o file                                                       |
     *     @note After this part we finished compiling file. @see step 2         |                                                    |
     *                                                                           |
     * 3.  @param All .o files got from step 2                                   |
     *     @brief Linker, static part                                            |
     *     @return On Windows - .exe file, On Linux: ELF executable file         |
     *     @note Windows & Linux have diffrent executables files                 |
     ****************************************************************************/
    void MainFlow();

    // TODO: Replace this method logic with Desrializer class call
    // @brief Desrialize input file path into wstring file path.
    // @param argc & argv: Command Line Arguments.
    // @return Encoded file path as wstring.
    std::wstring DesriralizeCLI(int argc, const char* argv[]);

private:
    std::wstring m_filePath;
    ParserLogic m_parserLogic;
};