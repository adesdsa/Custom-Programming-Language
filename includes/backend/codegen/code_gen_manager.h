#pragma once

#include "includes/shared/common/ast_helpers/decorated_ast.h"

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
};
