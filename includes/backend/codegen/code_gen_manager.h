#pragma once

#include <filesystem>

#include "includes/driver/compiler_options.h"
#include "includes/shared/common/ast_helpers/decorated_ast.h"

// Sub-Managers
#include "includes/backend/codegen/ir/ir_generator.h"
#include "includes/backend/codegen/ir/optimizations/optimization_manager.h"
#include "includes/backend/codegen/asm/asm_generator.h"
#include "includes/backend/assembler/assembler.h"
#include "includes/backend/linker/linker.h"

/**
    * Responsible for converting each source code into .o bin file.
    *
    * Each time diffrent worker generator will be called to convert 
    * from language a to language b. 
    * @example IRGenerator converts source code into LLVM like IR code.
*/
class CodeGenManager
{
public:
    CodeGenManager(const DecoratedAST& decoratedAST, const CompilerOptions& options);
    ~CodeGenManager() = default;

    /// @brief Runs IR generation, ASM emission, and assembly.
    /// @return Path to the produced artifact (.asm if stopAtAsm, .obj/.o otherwise).
    std::string MainFlow();

private:
    const DecoratedAST& m_decoratedAST;
    const CompilerOptions& m_options;
    ErrorManager& m_errorManager;
};
