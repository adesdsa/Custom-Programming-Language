#pragma once

#include <vector>
#include <string>
#include <exception>
#include <thread>

#include "includes/driver/compiler_options.h"
#include "includes/shared/utils/system_utils.h"
#include "includes/shared/utils/thread_pool.h"
#include "includes/frontend/parser/parser_logic.h"
#include "includes/frontend/parser/export_collector.h"
#include "includes/frontend/semantic_analysis/semantic_analysis_logic.h"
#include "includes/backend/codegen/code_gen_manager.h"
#include "includes/backend/linker/linker.h"

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
    /// @brief Construct the orchestrator with pre-parsed compiler options.
    /// Any UI (CLI, GUI, etc.) should build a CompilerOptions, validate it,
    /// and pass it here.
    explicit CompilerOrchestrator(CompilerOptions options);

    /*****************************************************************************
     * @brief Runs the complete compilation pipeline.                            |
     *                                                                           |
     * Calls Init() first, then for each source file:                            |
     *                                                                           |
     * 1. Lexer + Parser  (@see includes/frontend/parser)                        |
     *    @param  file_path  Absolute path to a .picoc source file               |
     *    @return AST                                                            |
     *                                                                           |
     * 2. Semantic Analysis  (@see includes/frontend/semantic_analysis)           |
     *    @param  AST                                                            |
     *    @return DecoratedAST (AST + VarTable Tree + FuncTable)                 |
     *                                                                           |
     * 3. IR Generation + Optimization  (@see includes/backend/codegen)          |
     *    @param  DecoratedAST                                                   |
     *    @return Optimized IR -> x86 .asm file                                  |
     *                                                                           |
     * 4. Assembler (NASM)                                                       |
     *    @param  .asm file                                                      |
     *    @return .obj file                                                      |
     *                                                                           |
     * 5. Linker                                                                 |
     *    @param  All .obj files from step 4                                     |
     *    @return Native executable                                              |
     *      - PE  (.exe) on Windows                                              |
     *      - ELF on Linux                                                       |
     ****************************************************************************/
    void MainFlow();

private:
    CompilerOptions m_options;

    /**
     * @brief Setup Compiler Orchestrator values
     */
    void Init();
};
