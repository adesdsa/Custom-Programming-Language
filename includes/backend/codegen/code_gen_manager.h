#pragma once

#include <filesystem>

#include "includes/shared/common/ast_helpers/decorated_ast.h"

// Sub-Managers
#include "includes/backend/codegen/ir/ir_generator.h"
#include "includes/backend/codegen/ir/optimizations/optimization_manager.h"
#include "includes/backend/codegen/asm/targets/x86_64emitter.h"
#include "includes/backend/codegen/asm/targets/x86_emitter.h"
#include "includes/backend/codegen/assembler.h"
#include "includes/backend/codegen/linker.h"

#include "includes/shared/common/target_architecture.h"

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

    void MainFlow();  // TODO: Change return value based on .o files implomention

private:
    const DecoratedAST& m_decoratedAST;
    ErrorManager& m_errorManager;
};
