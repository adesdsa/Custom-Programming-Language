#pragma once

#include <filesystem>

#include "includes/shared/common/ast_helpers/decorated_ast.h"

// Sub-Managers
#include "includes/backend/codegen/ir/ir_generator.h"
#include "includes/backend/codegen/ir/optimizations/optimization_manager.h"
#include "includes/backend/codegen/asm/asm_generator.h"

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
    CodeGenManager(const DecoratedAST& decoratedAST);
    ~CodeGenManager() = default;

    void MainFlow();

private:
    const DecoratedAST& m_decoratedAST;
    ErrorManager& m_errorManager;
};
