#include "includes/frontend/semantic_analysis/semantic_analysis_visitor.h"

SemanticAnalysisVisitor::SemanticAnalysisVisitor(
    const EntityLogger& entityLogger,
    VarTableNode& varTable,
    FuncTable& functable
) :
    m_entityLogger(entityLogger),
    m_errorManager(ErrorManager::GetInstance()),
    m_varTable(varTable),
    m_funcTable(functable),
    m_varFactory(entityLogger),
    m_funcFactory(entityLogger),
    m_currentScope(&varTable),
    m_lastExpressionType(ExpressionType::UNKNOWN),
    m_pendingAssignment(false),
    m_assignmentLine(0),
    m_assignmentColumn(0),
    m_identifierError(false),
    m_pendingBlockKind(SemanticCheckerFacade::BlockKind::Function),
    m_hasPendingBlock(false),
    m_bStartedFuncDefinition(false),
    m_bStartedFuncDeclaration(false)
{
    this->InitializeHandlers();
}

void SemanticAnalysisVisitor::VisitVarDeclaration(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    // saves var data
    const std::string& varName = ASTExtractor::ExtractVarName(node);
    Type varType = ASTExtractor::ExtractVarType(node);
    AccessRights varAccessRight = ASTExtractor::ExtractVarAccessRight(node);
    Scope varScope = ASTExtractor::ExtractVarScope(node);

    if (SemanticCheckerFacade::CheckVarExistence(*this->m_currentScope, varName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Variable '" + varName + "' is already declared in this scope",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Check if variable already declared, remember no shadowing allowed!"
            )
        );

        return;  // @note: stop logic here, no need to re-insert var again
    }

    if (this->m_bStartedFuncDeclaration) return; // If we are in func declaration var's arent saved in this scope.

    VarMetaData var = this->m_varFactory.CreateVarMetadata(
        varName, varType, varAccessRight, varScope
    );

    // `global` variables go to the root scope (visible from any function).
    // `var` variables go to the function scope root (visible throughout the function).
    // `let` variables go to the current block scope.
    VarTableNode* targetScope;
    if (varScope == Scope::GLOBAL)
    {
        targetScope = &this->m_varTable;
    }
    else if (varScope == Scope::VAR && this->m_functionScopeRoot != nullptr)
    {
        targetScope = this->m_functionScopeRoot;
    }
    else
    {
        targetScope = this->m_currentScope;
    }

    if (!targetScope->InsertVar(var))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::INTERNAL_ERROR,
                "Failed to insert variable '" + varName + "'",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "This is an internal error, please report it"
            )
        );
    }
}

void SemanticAnalysisVisitor::VisitIdentifier(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished())
    {
        this->m_identifierError = true;
        return;
    }
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty())
    {
        this->m_identifierError = true;
        return;
    }
    
    const std::string& identifierName = tokens[IDENTIFIER_NAME_INDEX].GetValue();
    
    if (!SemanticCheckerFacade::CheckVarExistence(*this->m_currentScope, identifierName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Cannot assign to undefined variable '" + identifierName + "'",
                tokens[IDENTIFIER_NAME_INDEX].GetLine(),
                tokens[IDENTIFIER_NAME_INDEX].GetColumn(),
                "Declare the variable before assigning to it"
            )
        );
        this->SignalFailureForNextHandlers();
        return;
    }
    
    auto var = this->m_currentScope->FindVar(identifierName);
    if (var.has_value())
    {
        const VarMetaData& varMetadata = var.value();
        
        if (varMetadata.accessRights == AccessRights::CONST_ACCESS)
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::ACCESS_RIGHT_ERROR,
                    "Cannot assign to constant variable '" + identifierName + "'",
                    tokens[IDENTIFIER_NAME_INDEX].GetLine(),
                    tokens[IDENTIFIER_NAME_INDEX].GetColumn(),
                    "Constant variables cannot be modified after declaration"
                )
            );
            this->SignalFailureForNextHandlers();
            return;
        }
        
        else if (varMetadata.accessRights == AccessRights::READ_ONLY_ACCESS)
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::ACCESS_RIGHT_ERROR,
                    "Cannot assign to read-only variable '" + identifierName + "'",
                    tokens[IDENTIFIER_NAME_INDEX].GetLine(),
                    tokens[IDENTIFIER_NAME_INDEX].GetColumn(),
                    "Read-only variables cannot be modified"
                )
            );
            this->SignalFailureForNextHandlers();
            return;
        }
    }
    
    this->m_lastIdentifierName = identifierName;
    this->m_identifierError = false;
}

void SemanticAnalysisVisitor::VisitIdentifierValue(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished()) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& identifierName = tokens[IDENTIFIER_NAME_INDEX].GetValue();
    
    if (!SemanticCheckerFacade::CheckVarExistence(*this->m_currentScope, identifierName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Undefined variable '" + identifierName + "' used in expression",
                tokens[IDENTIFIER_NAME_INDEX].GetLine(),
                tokens[IDENTIFIER_NAME_INDEX].GetColumn(),
                "Declare the variable before using it"
            )
        );
    }

    if (this->m_pendingForConditionCheck)
    {
        this->m_forExpressionCount++;

        if (this->m_forExpressionCount == 2)
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "for loop condition must be a boolean expression",
                    this->m_forConditionLine,
                    static_cast<uint16_t>(this->m_forConditionColumn),
                    "Provide a boolean condition"
                )
            );
            this->m_pendingForConditionCheck = false;
            this->m_forExpressionCount = 0;
        }
    }

    HandleSingleCommandAssignement(node);
}

/*
This method is the middle step in the assignment flow. 
It doesn't do the actual type check it just marks that an assignment is pending

x = 5
│   │
│   └── EXPRESSION: We need this type (int/bool/etc)
│
└── ASSIGNMENT: We're here, but don't have the expression yet

So we just set checks for VisitExpression to do later
*/
void SemanticAnalysisVisitor::VisitAssignment(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished())
    {
        ResetAssignmentState();
        return;
    }
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty())
    {
        ResetAssignmentState();
        return;
    }
    
    if (this->m_identifierError || this->m_lastIdentifierName.empty())
    {
        ResetAssignmentState();
        return;
    }

    // tell VisitExpression to run type check
    this->m_pendingAssignment = true;
    this->m_assignmentLine = tokens[FIRST_TOKEN_INDEX].GetLine();
    this->m_assignmentColumn = tokens[FIRST_TOKEN_INDEX].GetColumn();
}

void SemanticAnalysisVisitor::VisitExpression(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished())
    {
        // if VisitAssignment method set checks and we dont have expression node
        if (this->m_pendingAssignment && !this->m_lastIdentifierName.empty())
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::SYNTAX_ERROR,
                    "Expected expression after assignment to '" + this->m_lastIdentifierName + "'",
                    this->m_assignmentLine,
                    this->m_assignmentColumn,
                    "Provide a valid expression for the assignment"
                )
            );
        }
        ResetAssignmentState();
        this->m_lastExpressionType = ExpressionType::UNKNOWN;
        return;
    }
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    ExpressionType expressionType = SemanticCheckerFacade::CheckExpression(
        node->command,
        this->m_funcTable,
        *this->m_currentScope
    );

    if (expressionType == ExpressionType::UNKNOWN)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Invalid expression: type mismatch or undefined identifier/function",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Ensure all variables and functions are defined, and operand types are compatible"
            )
        );
    }
    this->m_lastExpressionType = expressionType;

    if (this->m_pendingForConditionCheck)
    {
        this->m_forExpressionCount++;
        
        if (this->m_forExpressionCount == 2)
        {
            if (expressionType != ExpressionType::BOOLEAN)
            {
                this->m_errorManager.StoreError(
                    CompileTimeError(
                        ErrorTypes::TYPE_ERROR,
                        "for loop condition must be a boolean expression",
                        this->m_forConditionLine,
                        static_cast<uint16_t>(this->m_forConditionColumn),
                        "Provide a boolean condition"
                    )
                );
            }
            this->m_pendingForConditionCheck = false;
            this->m_forExpressionCount = 0;
        }
    }

    // no pending assignment to complete
    if (!this->m_pendingAssignment || this->m_lastIdentifierName.empty()) return;

    /// complete assignment check, @see VisitAssignment docs
    if (
        expressionType != ExpressionType::UNKNOWN &&
        !SemanticCheckerFacade::CheckAssignment(
            this->m_lastIdentifierName,
            expressionType,
            *this->m_currentScope)
        )
    {
        auto var = this->m_currentScope->FindVar(this->m_lastIdentifierName);

        std::string expectedType = "unknown";
        if (var.has_value())
        {
            expectedType = this->TypeToString(var->type);
        }
        
        std::string gotType = this->ExpressionTypeToString(expressionType);
        
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Type mismatch: cannot assign '" + gotType + "' to variable '" + 
                    this->m_lastIdentifierName + "' of type '" + expectedType + "'",
                this->m_assignmentLine,
                this->m_assignmentColumn,
                "Ensure the expression type matches the variable type"
            )
        );
    }

    ResetAssignmentState();
}

void SemanticAnalysisVisitor::VisitFuncDeclaration(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[FUNCTION_NAME_TOKEN_INDEX].GetValue();
    
    if (SemanticCheckerFacade::CheckFuncDeclaration(this->m_funcTable, funcName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Function '" + funcName + "' is already declared",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Function names must be unique"
            )
        );
    }

    Type returnType = ASTExtractor::ExtractFuncReturnType(node);
    const std::vector<VarMetaData>& params = ASTExtractor::ExtractFuncParams(node, this->m_varFactory);
    
    FuncMetaData func = this->m_funcFactory.CreateFuncMetadata(
        funcName,
        returnType,
        params,
        tokens[FIRST_TOKEN_INDEX].GetLine(),
        tokens[FIRST_TOKEN_INDEX].GetColumn()
    );
    
    if (!this->m_funcTable.InsertFunction(std::move(func)))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::INTERNAL_ERROR,
                "Failed to register function '" + funcName + "'",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "This is an internal error, please report it"
            )
        );
    }

    // We set this flag only if it's only declaration and not both declaration and definition.
    if (!this->m_bStartedFuncDefinition)
    {
        this->m_bStartedFuncDeclaration = true;
    }
}

void SemanticAnalysisVisitor::VisitFuncDefinition(ASTNode* node)
{
    if (!node) return;

    this->m_bStartedFuncDefinition = true; // Setting the flag from the start incase its defination & decleration.
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[FUNCTION_NAME_TOKEN_INDEX].GetValue();
    
    if (SemanticCheckerFacade::CheckFuncDefinition(this->m_funcTable, funcName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Function '" + funcName + "' is already defined",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Function can only be defined once"
            )
        );

        return;
    }

    FuncMetaData* existingFunc = this->m_funcTable.FindFunctionByName(funcName);
    if (existingFunc != nullptr)
    {
        existingFunc->SetDefined();
    }
    else
    {
        // func not exist yet, both declaration & definition
        this->VisitFuncDeclaration(node);
        
        // After creating via VisitFuncDeclaration, mark as defined too
        FuncMetaData* newFunc = this->m_funcTable.FindFunctionByName(funcName);
        if (newFunc != nullptr)
        {
            newFunc->SetDefined();
        }
    }

    this->m_currentFunctionName = funcName;
    
    // signal VisitBlockStart what kind of block is coming
    this->m_pendingBlockKind = SemanticCheckerFacade::BlockKind::Function;
    this->m_hasPendingBlock = true;

    // new var block starts now to save arguements.
    this->m_currentScope = this->m_currentScope->CreateBlock();
    this->m_functionScopeRoot = this->m_currentScope;

    FuncMetaData* funcMeta = this->m_funcTable.FindFunctionByName(funcName);
    if (funcMeta != nullptr)
    {
        funcMeta->SetFunctionScopeRoot(this->m_currentScope);
    }
}

void SemanticAnalysisVisitor::VisitFuncCall(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished()) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[FUNCTION_NAME_TOKEN_INDEX].GetValue();
    
    if (!SemanticCheckerFacade::CheckFuncDeclaration(this->m_funcTable, funcName))
    {
        // Lazily register built-in functions on first use
        this->m_funcTable.RegisterBuiltins();

        // Re-check after registering built-ins
        if (!SemanticCheckerFacade::CheckFuncDeclaration(this->m_funcTable, funcName))
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::NAME_ERROR,
                    "Undefined function '" + funcName + "'",
                    tokens[FIRST_TOKEN_INDEX].GetLine(),
                    tokens[FIRST_TOKEN_INDEX].GetColumn(),
                    "Function must be declared before calling"
                )
            );
            return;
        }
    }
    
    std::vector<ExpressionType> argTypes;

    // for each argument passed we will evaluate it type
    for (ASTNode* arg = node->nodeFirstChild; arg != nullptr; arg = arg->nodeNextSibling)
    {
        argTypes.push_back(this->EvaluateNodeType(arg));
    }
    
    if (!SemanticCheckerFacade::CheckFuncArguments(argTypes, funcName, this->m_funcTable))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::ARGUMENT_ERROR,
                "Invalid arguments for function '" + funcName + "'",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Check the number and types of arguments"
            )
        );
    }

    HandleSingleCommandAssignement(node);
}

void SemanticAnalysisVisitor::VisitIf(ASTNode* node)
{
    this->VisitIfOrElseIfOrWhile(node);
}

void SemanticAnalysisVisitor::VisitElif(ASTNode* node)
{
    this->VisitIfOrElseIfOrWhile(node);
}

void SemanticAnalysisVisitor::VisitElse(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished()) return;
    
    // signal VisitBlockStart what kind of block is coming
    this->m_pendingBlockKind = SemanticCheckerFacade::BlockKind::Else;
    this->m_hasPendingBlock = true;
}

void SemanticAnalysisVisitor::VisitWhile(ASTNode* node)
{
    this->VisitIfOrElseIfOrWhile(node);
}

void SemanticAnalysisVisitor::VisitFor(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished()) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    
    /**
        @brief Helper Flags:

        * @struct for loop:

        * (VAR_DECLERATION/IDENTIFIER) -> ASSIGNMENT -> EXPRESSION/FUNC_CALL/etc
        * <------------------------- init part of the for ---------------------->

        *              EXPRESSION
        * <------ condition part of the for ----->

        *   IDENTIFIER -> ASSIGNMENT -> EXPRESSION/FUNC_CALL/etc
        * <---------- Increasement part of the for --------------->
        
        * @note We only need to the condition part because init & inc part just need
        * to be valid but have no effect on for so it would be handle in their visit
        * however, the condition part just checks that it must be a boolean
    */

    bool passedFirstAssignment = false;
    bool passedInitValue = false;
    ASTNode* conditionNode = nullptr;
    
    for (ASTNode* child = node->nodeFirstChild; child != nullptr; child = child->nodeNextSibling)
    {        
        if (!passedFirstAssignment)
        {
            if (child->command.GetType() == CommandTypes::ASSIGNMENT)
            {
                passedFirstAssignment = true;
            }
        }
        else if (!passedInitValue)
        {
            passedInitValue = true;
        }
        else if (!conditionNode)
        {
            conditionNode = child;
            break;  // found the condition part of the for loop
        }
    }
    
    this->m_pendingForConditionCheck = true;
    this->m_forConditionLine = tokens.empty() ? 0 : tokens[FIRST_TOKEN_INDEX].GetLine();
    this->m_forConditionColumn = tokens.empty() ? 0 : tokens[FIRST_TOKEN_INDEX].GetColumn();
    this->m_forExpressionCount = 0;
    
    // signal VisitBlockStart what kind of block is coming
    this->m_pendingBlockKind = SemanticCheckerFacade::BlockKind::Loop;
    this->m_hasPendingBlock = true;
}

void SemanticAnalysisVisitor::VisitReturn(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished()) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    size_t line = tokens.empty() ? 0 : tokens[FIRST_TOKEN_INDEX].GetLine();
    size_t column = tokens.empty() ? 0 : tokens[FIRST_TOKEN_INDEX].GetColumn();
    
    if (!SemanticCheckerFacade::CheckReturnInsideFunc())
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::STATEMENT_OUT_OF_BLOCK_ERROR,
                "return statement outside of function",
                line,
                column,
                "Return can only be used inside a function body"
            )
        );
    }

    const auto& exprType = this->EvaluateNodeType(node->nodeNextSibling);
    if (exprType == ExpressionType::UNKNOWN) return;

    if (
        !SemanticCheckerFacade::CheckFunctionReturnType(
            exprType,
            this->m_currentFunctionName,
            this->m_funcTable
        )
    )
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "return type mismatch in function '" + this->m_currentFunctionName + "'",
                line,
                column,
                "Ensure return type matches function declaration"
            )
        );
    }
}

void SemanticAnalysisVisitor::VisitControlStatement(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished()) return;
    
    if (!SemanticCheckerFacade::CheckControlStatement())
    {    
        const auto& tokens = node->command.GetTokens();
        if (tokens.empty()) return;

        const std::string& statement = tokens[STATEMENT_TOKEN_INDEX].GetValue();
        size_t line = tokens[FIRST_TOKEN_INDEX].GetLine();
        size_t column = tokens[FIRST_TOKEN_INDEX].GetColumn();

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::STATEMENT_OUT_OF_BLOCK_ERROR,
                "'" + statement + "' statement outside of loop",
                line,
                column,
                statement + " can only be used inside a loop"
            )
        );
    }
}

void SemanticAnalysisVisitor::VisitBlockStart(ASTNode* node)
{
    if (!node) return;

    // If we are in func definition the block was already created.
    if (!this->m_bStartedFuncDefinition)
    {
        this->m_currentScope = this->m_currentScope->CreateBlock();
    }
    else
    {
        this->m_bStartedFuncDefinition = false;
    }
    
    bool calledEnteredBlock = false;
    
    // enter block in facade if a control structure set pending block kind
    if (this->m_hasPendingBlock)
    {
        calledEnteredBlock = this->HandleEnteredBlockCheck(node, this->m_pendingBlockKind);
        this->m_hasPendingBlock = false;
    }
    
    // track whether this block level called EnteredBlock for matching ExitedBlock call
    this->m_blockEnteredStack.push(calledEnteredBlock);
}

void SemanticAnalysisVisitor::VisitBlockEnd(ASTNode* node)
{
    if (!node) return;
    
    // Exit scope - move to parent
    const auto& parent = this->m_currentScope->GetParent();
    if (parent != nullptr)
    {
        this->m_currentScope = parent;
    }

    // Only call ExitedBlock if we called EnteredBlock for this block level
    bool shouldCallExitedBlock = !this->m_blockEnteredStack.empty() && 
                                  this->m_blockEnteredStack.top();
    
    if (!this->m_blockEnteredStack.empty())
    {
        this->m_blockEnteredStack.pop();
    }
    
    if (shouldCallExitedBlock)
    {
        // Exit block in facade (handles return path analysis)
        this->HandleExitedBlockCheck(node);
    }
}

void SemanticAnalysisVisitor::VisitEndOfCommand(ASTNode* node)
{
    if (this->m_bStartedFuncDeclaration)
    {
        this->m_bStartedFuncDeclaration = false; // Flag reset.
    }
}

void SemanticAnalysisVisitor::VisitSizeof(ASTNode* node)
{
    // skip, no logic needed
}

void SemanticAnalysisVisitor::VisitVoidCommand(ASTNode* node)
{
    // skip, no logic needed
}

void SemanticAnalysisVisitor::VisitPass(ASTNode* node)
{
    // skip, no logic needed
}

void SemanticAnalysisVisitor::VisitEof(ASTNode* node)
{
    // skip, no logic needed
}

void SemanticAnalysisVisitor::VisitIfOrElseIfOrWhile(ASTNode* node)
{
    if (!node || SemanticCheckerFacade::IsBlockFinished()) return;
    
    // IF/ELSE IF/WHILE ()
    //  condition() <-------- node->nodeFirstChild
    ASTNode* condition = node->nodeFirstChild;
    
    if (!condition) return;

    const auto& tokens = condition->command.GetTokens();
    if (tokens.empty()) return;
    const auto& conditionTokens = tokens[CONDITION_TOKEN_INDEX];

    if (condition->command.GetType() == CommandTypes::EXPRESSION)
    {
        ExpressionType conditionType = EvaluateNodeType(condition);
        if (conditionType != ExpressionType::BOOLEAN)
        { 
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Condition must be boolean. Following expression is not a boolean",
                    conditionTokens.GetLine(),
                    conditionTokens.GetColumn(),
                    "Provide valid boolean inside the brackets"
                )
            );
        }
    }

    else if (condition->command.GetType() == CommandTypes::IDENTIFIER_VALUE)
    {
        const std::string& varName = condition->command.GetTokens()[VAR_NAME_TOKEN_INDEX].GetValue();
        auto var = this->m_currentScope->FindVar(varName);
    
        if (var.has_value() && var->type != Type::BOOL_TYPE)
        {       
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Condition must be boolean. Following variable is not a boolean",
                    conditionTokens.GetLine(),
                    conditionTokens.GetColumn(),
                    "Provide valid boolean inside the if brackets"
                )
            );
        }
    }

    else if (condition->command.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        const std::string& calledFuncName = condition->command.GetTokens()[FUNCTION_NAME_TOKEN_INDEX].GetValue();
        const auto func = this->m_funcTable.FindFunctionByName(calledFuncName);

        if (func && func->GetRType() != Type::BOOL_TYPE)
        { 
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Condition must be boolean. Following function call do not return a boolean",
                    conditionTokens.GetLine(),
                    conditionTokens.GetColumn(),
                    "Provide valid boolean inside the if brackets"
                )
            );
        }
    }

    else
    {
        this->m_entityLogger.Error(
            "Invalid condition at line: " + std::to_string(conditionTokens.GetLine())
        );
    }

    // Determine block kind based on node type
    const auto& nodeType = node->command.GetType();
    SemanticCheckerFacade::BlockKind blockKind = SemanticCheckerFacade::BlockKind::If;

    if (nodeType == CommandTypes::ELIF)
    {
        blockKind = SemanticCheckerFacade::BlockKind::Elif;
    }
    else if (nodeType == CommandTypes::WHILE)
    {
        blockKind = SemanticCheckerFacade::BlockKind::Loop;
    }

    // signal VisitBlockStart what kind of block is coming
    this->m_pendingBlockKind = blockKind;
    this->m_hasPendingBlock = true;
}

void SemanticAnalysisVisitor::SignalFailureForNextHandlers() noexcept
{
    this->m_identifierError = true;
    this->m_lastIdentifierName.clear();
}

void SemanticAnalysisVisitor::ResetAssignmentState() noexcept
{
    this->m_lastIdentifierName.clear();
    this->m_pendingAssignment = false;
    this->m_assignmentLine = 0;
    this->m_assignmentColumn = 0;
    this->m_identifierError = false;
}

std::string SemanticAnalysisVisitor::TypeToString(
    Type type
) const noexcept
{
    switch (type)
    {
        case Type::BOOL_TYPE:            return "bool";
        case Type::INT_TYPE:             return "int";
        case Type::DOUBLE_TYPE:          return "double";
        case Type::UNSIGNED_INT_TYPE:    return "unsigned int";
        case Type::VOID_TYPE:            return "void";
        default:                         return "unknown";
    }
}

std::string SemanticAnalysisVisitor::ExpressionTypeToString(
    ExpressionType expressionType
) const noexcept
{
    switch (expressionType)
    {
        case ExpressionType::BOOLEAN:      return "boolean";
        case ExpressionType::MATHEMATICAL: return "numeric";
        case ExpressionType::VOID_TYPE:    return "void";
        default:                           return "unknown";
    }
}

ExpressionType SemanticAnalysisVisitor::EvaluateNodeType(ASTNode* node) const noexcept
{
    if (!node) return ExpressionType::UNKNOWN;
    
    std::string_view nodeType = node->command.GetType();
    
    if (nodeType == CommandTypes::IDENTIFIER_VALUE)
    {
        const auto& varTokens = node->command.GetTokens();
        if (varTokens.empty())
        {
            return ExpressionType::UNKNOWN;
        }

        const std::string& varName = varTokens[VAR_NAME_TOKEN_INDEX].GetValue();
        auto var = this->m_currentScope->FindVar(varName);

        if (var.has_value()) 
        {
            return SemanticCheckerFacade::TypeToExpressionType(var->type);
        }
        else
        {
            return ExpressionType::UNKNOWN;
        }
    }
    
    else if (nodeType == CommandTypes::EXPRESSION)
    {
        return SemanticCheckerFacade::CheckExpression(
            node->command, 
            this->m_funcTable, 
            *this->m_currentScope
        );
    }
    
    else if (nodeType == CommandTypes::FINISHED_FUNC_CALL)
    {       
        const std::vector<Token>& tokens = node->command.GetTokens();
        if (tokens.empty()) 
        {
            return ExpressionType::UNKNOWN;
        }
    
        const std::string& calledFuncName = tokens[FUNCTION_NAME_TOKEN_INDEX].GetValue();
        FuncMetaData* func = this->m_funcTable.FindFunctionByName(calledFuncName);
        if (!func) 
        {
            return ExpressionType::UNKNOWN;
        }

        return SemanticCheckerFacade::TypeToExpressionType(func->GetRType());
    }
    
    return ExpressionType::UNKNOWN;
}

bool SemanticAnalysisVisitor::HandleEnteredBlockCheck(
        ASTNode* node, 
        SemanticCheckerFacade::BlockKind blockkind
    )
{
    bool isVoidReturnType = false;
    
    // Only need function lookup for Function blocks to determine return type
    if (blockkind == SemanticCheckerFacade::BlockKind::Function)
    {
        const auto* func = this->m_funcTable.FindFunctionByName(this->m_currentFunctionName);
        if (!func)
        {
            // Can't enter function block without function metadata
            // This shouldn't happen if VisitFuncDefinition works correctly
            return false;
        }
        isVoidReturnType = func->GetRType() == Type::VOID_TYPE;
    }
    
    // Call EnteredBlock for ALL block types (Function, If, Else, Elif, Loop)
    if (!SemanticCheckerFacade::EnteredBlock(blockkind, isVoidReturnType))
    {
        const auto& tokens = node->command.GetTokens();
        if (!tokens.empty())
        {
            const auto& errorTokens = tokens[FIRST_TOKEN_INDEX];

            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::STATEMENT_OUT_OF_BLOCK_ERROR,
                    "Invalid usage of else if/else keywords without matching if keyword",
                    errorTokens.GetLine() - 1,
                    errorTokens.GetColumn(),
                    "else if/else keyword are only allowed after usage of if keyword"
                )
            );
        }
        // EnteredBlock failed, but we still called it so it may have partially modified state
        // Return true because EnteredBlock was called
    }
    
    return true;  // EnteredBlock was called
}

void SemanticAnalysisVisitor::HandleExitedBlockCheck(ASTNode* node) const
{
    if (!SemanticCheckerFacade::ExitedBlock())
    {
        const auto& tokens = node->command.GetTokens();
        if (tokens.empty()) return;
        const auto& errorTokens = tokens[FIRST_TOKEN_INDEX];

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::FUNC_MISSING_RETURN_ERROR,

                "Not all paths of non void function " + 
                this->m_currentFunctionName + 
                " have a return statement",

                errorTokens.GetLine(),
                errorTokens.GetColumn(),

                "Non void functions must return a value at all end-points"
            )
        );
    }
}

void SemanticAnalysisVisitor::HandleSingleCommandAssignement(ASTNode* node)
{
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;

    ExpressionType expressionType = SemanticCheckerFacade::CheckExpression(
        node->command,
        this->m_funcTable,
        *this->m_currentScope
    );

    if (expressionType == ExpressionType::UNKNOWN)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Invalid expression: type mismatch or undefined identifier/function",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Ensure all variables and functions are defined, and operand types are compatible"
            )
        );
    }
    this->m_lastExpressionType = expressionType;

    // no pending assignment to complete
    if (!this->m_pendingAssignment || this->m_lastIdentifierName.empty()) return;

    /// complete assignment check, @see VisitAssignment docs
    if (
        expressionType != ExpressionType::UNKNOWN &&
        !SemanticCheckerFacade::CheckAssignment(
            this->m_lastIdentifierName,
            expressionType,
            *this->m_currentScope)
        )
    {
        auto var = this->m_currentScope->FindVar(this->m_lastIdentifierName);

        std::string expectedType = "unknown";
        if (var.has_value())
        {
            expectedType = this->TypeToString(var->type);
        }

        std::string gotType = this->ExpressionTypeToString(expressionType);

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Type mismatch: cannot assign '" + gotType + "' to variable '" +
                this->m_lastIdentifierName + "' of type '" + expectedType + "'",
                this->m_assignmentLine,
                this->m_assignmentColumn,
                "Ensure the expression type matches the variable type"
            )
        );
    }

    ResetAssignmentState();
}
