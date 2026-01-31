#pragma once

#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/shared/common/ast_helpers/ast_iterator.h"
#include "includes/shared/common/ast_helpers/decorated_ast.h"
#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/ir/ir_visitor.h"

/**
    * @brief Generate picoC code into LLVM IR code
*/
class IRGenerator
{
public:
    IRGenerator(const DecoratedAST& decoratedAST);
    ~IRGenerator() = default;

    /*****************************************************************************
     * @brief Runs Whole IR Generation Pipeline:                                 |
     *                                                                           |
     * INPUT: `decoratedAST`                                                     |
     * OUTPUT: LLVM like IR stored inside LLVM IRMoudle                          |
     *                                                                           |
     *                                                                           |
     * 1.0  @see `IRGenerator`                                                   |
     *      @param `DecoratedAST`                                                |
     *      @brief Iterating through each node in the AST and preform job:       |                                        |
     *      @return LLVM IR stored inside LLVM IRMoudle                          |                                 |
     *                                                                           |
     * 1.1. @see `IRVisitor`                                                     |
     *      @param ASTNode*                                                      |
     *      @brief `Detect right handler for the node and call it                |                                        |
     *      @return None (Save changes at IRMoudle)                              |
     *                                                                           |
     *  
     * 1.1.1 @see `IRBuilder`                                                    |
     *     @param ASTNode*                                                       |
     *     @brief Builds IR code based on the command in the ASTNode             |
     *     @return None (Save changes at IRMoudle)                               |                                                          |
     *                                                                           |
     * 1.1.2 @see `Optimizer`                                                    |
     *     @param IR code from previous step                                     |
     *     @brief Change the IR code by optimizing it                            |
     *     @return None (Change the code at IRMoudle)                            |
     ****************************************************************************/
    IRModule& MainFlow();

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
    IRVisitor m_irVisitor;  // generate each node into ir

    /**********************\
    |        Output        |
    \**********************/
    IRModule m_irMoudle;  // the entire ir program
};
