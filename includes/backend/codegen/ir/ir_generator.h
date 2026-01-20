#pragma once

#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/shared/common/ast_helpers/ast_iterator.h"
#include "includes/shared/common/ast_helpers/decorated_ast.h"
#include "includes/backend/codegen/ir/ir_templates/ir_module.h"

/**
    * @brief Generate picoC code into LLVM IR code
*/
class IRGenerator
{
public:
    IRGenerator(const DecoratedAST& decoratedAST);
    ~IRGenerator() = default;

    /// @note Please provide full pipeline docs
    const IRModule& MainFlow();

private:
    /**********************\
    |        Utils         |
    \**********************/
    EntityLogger m_IRLogger;
    ErrorManager& m_errorManager;

    /**********************\
    |        Jobs          |
    \**********************/
    ASTIterator m_astIterator;
    // IRVisitor m_irVisitor;  // generate each node into ir

    /**********************\
    |        Output        |
    \**********************/
    IRModule m_irMoudle;  // the entire ir program
};
