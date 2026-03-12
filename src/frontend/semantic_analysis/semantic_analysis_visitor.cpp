#include "includes/frontend/semantic_analysis/semantic_analysis_visitor.h"

#include <filesystem>
#include <unordered_set>

SemanticAnalysisVisitor::SemanticAnalysisVisitor(
    const EntityLogger& entityLogger,
    VarTableNode& varTable,
    FuncTable& functable,
    const ProjectSymbolTable& projectSymbolTable
) :
    m_projectSymbolTable(projectSymbolTable),
    m_facade(),
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

    if (this->m_facade.CheckVarExistence(*this->m_currentScope, varName))
    {
        if (varScope == Scope::GLOBAL || varScope == Scope::EXPORT) return;

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Variable '" + varName + "' is already declared in this scope",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Variable is already declared in this scope (shadowing is not allowed)"
            )
        );

        return;  // @note: stop logic here, no need to re-insert var again
    }

    if (this->m_bStartedFuncDeclaration) return; // If we are in func declaration var's arent saved in this scope.

    VarMetaData var = this->m_varFactory.CreateVarMetadata(
        varName, varType, varAccessRight, varScope, std::nullopt
    );

    // `global` variables go to the root scope (visible from any function).
    // `var` variables go to the function scope root (visible throughout the function).
    // `let` variables go to the current block scope.
    VarTableNode* targetScope;
    if (varScope == Scope::GLOBAL || varScope == Scope::EXPORT)
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

    // Set for assignment check when next sibling is ASSIGNMENT (e.g. let x: T = expr)
    this->m_lastIdentifierName = varName;
    this->m_isVarInitialization = true;
}

void SemanticAnalysisVisitor::VisitArrayDeclaration(ASTNode* node)
{
    if (!node) return;

    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;

    const std::string& varName = ASTExtractor::ExtractVarName(node);
    Type varType = ASTExtractor::ExtractVarType(node);
    AccessRights varAccessRight = ASTExtractor::ExtractVarAccessRight(node);
    Scope varScope = ASTExtractor::ExtractVarScope(node);
    std::optional<size_t> arraySize = ASTExtractor::ExtractArraySize(node);

    if (varType == Type::VOID_TYPE)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Cannot declare array of void type",
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Use a valid element type (int, double, bool, char, string)"
            )
        );
        return;
    }

    if (!arraySize.has_value())
    {
        arraySize = ASTExtractor::ExtractArraySizeFromInitializer(node);
    }
    if (!arraySize.has_value())
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::SYNTAX_ERROR,
                "Array size must be a constant non-negative integer or inferred from initializer (e.g. int[] = [1,2,3])",
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Use a literal like 0, 10, a const expression, or int[] = [1,2,3] to infer from initializer"
            )
        );
        return;
    }

    if (this->m_facade.CheckVarExistence(*this->m_currentScope, varName))
    {
        if (varScope == Scope::GLOBAL || varScope == Scope::EXPORT) return;

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Variable '" + varName + "' is already declared in this scope",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Variable is already declared in this scope (shadowing is not allowed)"
            )
        );
        return;
    }

    if (this->m_bStartedFuncDeclaration) return;

    VarMetaData var = this->m_varFactory.CreateVarMetadata(
        varName, varType, varAccessRight, varScope, arraySize
    );

    VarTableNode* targetScope;
    if (varScope == Scope::GLOBAL || varScope == Scope::EXPORT)
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

    this->m_lastIdentifierName = varName;
    this->m_isVarInitialization = true;
}

void SemanticAnalysisVisitor::VisitIdentifier(ASTNode* node)
{
    if (!node || this->m_facade.IsBlockFinished())
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
    
    if (!this->m_facade.CheckVarExistence(*this->m_currentScope, identifierName))
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
    this->m_isVarInitialization = false;
    this->m_identifierError = false;
}

void SemanticAnalysisVisitor::VisitIdentifierValue(ASTNode* node)
{
    if (!node || this->m_facade.IsBlockFinished()) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& identifierName = tokens[IDENTIFIER_NAME_INDEX].GetValue();
    
    if (!this->m_facade.CheckVarExistence(*this->m_currentScope, identifierName))
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
    if (!node || this->m_facade.IsBlockFinished())
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
    if (!node || this->m_facade.IsBlockFinished())
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
    
    ExpressionType expressionType = this->m_facade.CheckExpression(
        node->command,
        this->m_funcTable,
        *this->m_currentScope
    );

    if (expressionType == ExpressionType::UNKNOWN)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Cannot evaluate expression — possible type mismatch or use of an undefined variable/function",
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
        !this->m_facade.CheckAssignment(
            this->m_lastIdentifierName,
            expressionType,
            *this->m_currentScope,
            this->m_isVarInitialization)
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
    
    if (this->m_facade.CheckFuncDeclaration(this->m_funcTable, funcName))
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
    
    if (this->m_facade.CheckFuncDefinition(this->m_funcTable, funcName))
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
        // verify the definition's signature matches the declaration's
        Type returnType = ASTExtractor::ExtractFuncReturnType(node);
        const std::vector<VarMetaData>& defParams = ASTExtractor::ExtractFuncParams(node, this->m_varFactory);
        FuncMetaData defMeta = this->m_funcFactory.CreateFuncMetadata(
            funcName, returnType, defParams,
            tokens[FIRST_TOKEN_INDEX].GetLine(),
            tokens[FIRST_TOKEN_INDEX].GetColumn()
        );

        if (!existingFunc->HasSameSignature(defMeta))
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Definition of function '" + funcName +
                        "' does not match its declaration",
                    tokens[FIRST_TOKEN_INDEX].GetLine(),
                    tokens[FIRST_TOKEN_INDEX].GetColumn(),
                    "Ensure the parameter types and return type match the declaration at line "
                        + std::to_string(existingFunc->GetDeclarationLine())
                )
            );
            return;
        }

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
    if (!node || this->m_facade.IsBlockFinished()) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[FUNCTION_NAME_TOKEN_INDEX].GetValue();
    
    if (!this->m_facade.CheckFuncDeclaration(this->m_funcTable, funcName))
    {
        // Lazily register built-in functions on first use
        this->m_funcTable.RegisterBuiltins();

        // Re-check after registering built-ins
        if (!this->m_facade.CheckFuncDeclaration(this->m_funcTable, funcName) &&
            this->m_funcTable.FindFunctionByName(funcName))
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

    static const std::unordered_set<std::string> ARRAYLIB_FUNCS = {
        "front", "back", "len", "find", "find_first_of", "find_last_of",
        "front_str", "back_str", "len_str", "find_str", "find_first_of_str", "find_last_of_str",
        "front_double", "back_double", "len_double", "find_double", "find_first_of_double", "find_last_of_double",
        "front_uint", "back_uint", "len_uint", "find_uint", "find_first_of_uint", "find_last_of_uint",
        "front_char", "back_char", "len_char", "find_char", "find_first_of_char", "find_last_of_char",
        "front_bool", "back_bool", "len_bool", "find_bool", "find_first_of_bool", "find_last_of_bool"
    };
    
    if (!this->m_facade.CheckFuncArguments(argTypes, funcName, this->m_funcTable) &&
        ARRAYLIB_FUNCS.count(funcName) == 0)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::ARGUMENT_ERROR,
                "Wrong number or types of arguments in call to '" + funcName + "'",
                tokens[FIRST_TOKEN_INDEX].GetLine(),
                tokens[FIRST_TOKEN_INDEX].GetColumn(),
                "Check that the argument count and types match the function declaration"
            )
        );
    }

    // arraylib functions require first arg to be an array
    if (ARRAYLIB_FUNCS.count(funcName) && node->nodeFirstChild)
    {
        const Command& firstArg = node->nodeFirstChild->command;
        if (firstArg.GetType() == CommandTypes::IDENTIFIER_VALUE)
        {
            const std::string& varName = firstArg[0].GetValue();
            auto var = this->m_currentScope->FindVar(varName);
            if (var && !var->arraySize.has_value())
            {
                this->m_errorManager.StoreError(
                    CompileTimeError(
                        ErrorTypes::TYPE_ERROR,
                        "'" + funcName + "' requires an array argument, but '" + varName + "' is not an array",
                        firstArg[0].GetLine(),
                        firstArg[0].GetColumn(),
                        "Pass an array variable (e.g. int[] or string[])"
                    )
                );
            }
        }
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
    if (!node || this->m_facade.IsBlockFinished()) return;
    
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
    if (!node || this->m_facade.IsBlockFinished()) return;
    
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
    this->m_currentScope = this->m_currentScope->CreateBlock();
    this->m_pendingForInitScope = true;

    // signal VisitBlockStart what kind of block is coming
    this->m_pendingBlockKind = SemanticCheckerFacade::BlockKind::Loop;
    this->m_hasPendingBlock = true;
}

void SemanticAnalysisVisitor::VisitReturn(ASTNode* node)
{
    if (!node || this->m_facade.IsBlockFinished()) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    size_t line = tokens.empty() ? 0 : tokens[FIRST_TOKEN_INDEX].GetLine();
    size_t column = tokens.empty() ? 0 : tokens[FIRST_TOKEN_INDEX].GetColumn();
    
    if (!this->m_facade.CheckReturnInsideFunc())
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
        !this->m_facade.CheckFunctionReturnType(
            exprType,
            this->m_currentFunctionName,
            this->m_funcTable
        )
    )
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Return type mismatch in function '" + this->m_currentFunctionName +
                    "' — returned value does not match declared return type",
                line,
                column,
                "Ensure the returned value type matches the function's declared return type"
            )
        );
    }
}

void SemanticAnalysisVisitor::VisitControlStatement(ASTNode* node)
{
    if (!node || this->m_facade.IsBlockFinished()) return;
    
    if (!this->m_facade.CheckControlStatement())
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

    // clear condition evaluation suppression set by VisitIfOrElseIfOrWhile.
    // at this point the AST walker has finished visiting all condition children.
    this->m_facade.SetSuppressDefaultExprCheck(false);

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
    bool hasForInitScope = this->m_pendingForInitScope;
    this->m_pendingForInitScope = false;
    
    // enter block in facade if a control structure set pending block kind
    if (this->m_hasPendingBlock)
    {
        calledEnteredBlock = this->HandleEnteredBlockCheck(node, this->m_pendingBlockKind);
        this->m_hasPendingBlock = false;
    }
    
    // track whether this block level called EnteredBlock for matching ExitedBlock call
    this->m_blockEnteredStack.push(calledEnteredBlock);
    this->m_forInitScopeStack.push(hasForInitScope);
}

void SemanticAnalysisVisitor::VisitBlockEnd(ASTNode* node)
{
    if (!node) return;
    
    const auto& parent = this->m_currentScope->GetParent();
    if (parent != nullptr)
    {
        this->m_currentScope = parent;
    }

    // if this block had a for-init wrapper scope, pop it too
    bool hadForInitScope = !this->m_forInitScopeStack.empty() &&
                            this->m_forInitScopeStack.top();
    if (!this->m_forInitScopeStack.empty())
    {
        this->m_forInitScopeStack.pop();
    }
    if (hadForInitScope)
    {
        const auto& forParent = this->m_currentScope->GetParent();
        if (forParent != nullptr)
        {
            this->m_currentScope = forParent;
        }
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

void SemanticAnalysisVisitor::VisitImportStatement(ASTNode* node)
{
    const auto& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    const auto& pathToken = tokens.back();
    std::string importPath = pathToken.GetValue();
    if (importPath.size() >= 2 && importPath.front() == '"' && importPath.back() == '"')
        importPath = importPath.substr(1, importPath.size() - 2);

    if (BuiltinLibraryRegistry::IsBuiltinLibrary(importPath))
    {
        HandleBuiltinLibraryImport(node, importPath);
        return;
    }

    std::string moduleName = std::filesystem::path(importPath).stem().string();
    auto fileExports = this->m_projectSymbolTable.Resolve(moduleName);
    if (fileExports == nullptr)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::UNPROCESSED_FILE_IMPORTED,
                "The file " + importPath + " wasn't parsed",
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Add the file as input"
            )
        );
        return;
    }

    auto imports = node->command.GetParams();
    const int importLine = static_cast<int>(tokens[0].GetLine());
    const int importColumn = static_cast<int>(tokens[0].GetColumn());

    if (imports.empty())
    {
        // wildcard import: import * from "file.picoc"
        for (const auto& function : fileExports->functions)
        {
            if (!this->m_funcTable.InsertFunction(this->m_funcFactory.CreateFuncMetadata(function)))
            {
                this->m_errorManager.StoreError(
                    CompileTimeError(
                        ErrorTypes::NAME_ERROR,
                        "Duplicate import: '" + function.name + "' was already imported",
                        importLine,
                        importColumn,
                        "Remove the duplicate import"
                    )
                );
            }
        }
        for (const auto& variable : fileExports->exportedVars)
        {
            if (!this->m_varTable.InsertVar(this->m_varFactory.CreateVarMetadata(variable)))
            {
                this->m_errorManager.StoreError(
                    CompileTimeError(
                        ErrorTypes::NAME_ERROR,
                        "Duplicate import: '" + variable.name + "' was already imported",
                        importLine,
                        importColumn,
                        "Remove the duplicate import"
                    )
                );
            }
        }
        return;
    }

    for (auto import : imports)
    {
        bool found = false;
        const std::string& symbolName = import.GetTokens()[0].GetValue();
        const int line = static_cast<int>(import.GetTokens()[0].GetLine());
        const int column = static_cast<int>(import.GetTokens()[0].GetColumn());

        for (auto function : fileExports->functions)
        {
            if (function.name == symbolName)
            {
                found = true;
                if (!this->m_funcTable.InsertFunction(this->m_funcFactory.CreateFuncMetadata(function)))
                {
                    this->m_errorManager.StoreError(
                        CompileTimeError(
                            ErrorTypes::NAME_ERROR,
                            "Duplicate import: '" + symbolName + "' was already imported",
                            line,
                            column,
                            "Remove the duplicate import"
                        )
                    );
                }
                break;
            }
        }

        for (auto variable : fileExports->exportedVars)
        {
            if (variable.name == symbolName)
            {
                found = true;
                if (!this->m_varTable.InsertVar(this->m_varFactory.CreateVarMetadata(variable)))
                {
                    this->m_errorManager.StoreError(
                        CompileTimeError(
                            ErrorTypes::NAME_ERROR,
                            "Duplicate import: '" + symbolName + "' was already imported",
                            line,
                            column,
                            "Remove the duplicate import"
                        )
                    );
                }
                break;
            }
        }

        if (!found)
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::NAME_ERROR,
                    "The file " + importPath + " doesn't export: " + symbolName,
                    node->command.GetTokens()[0].GetLine(),
                    node->command.GetTokens()[0].GetColumn(),
                    "Maybe you forgot to add export to the function/variable"
                )
            );
        }
    }
}

void SemanticAnalysisVisitor::HandleBuiltinLibraryImport(ASTNode* node, const std::string& libName)
{
    const auto& tokens = node->command.GetTokens();
    const auto* lib = BuiltinLibraryRegistry::GetLibrary(libName);
    if (!lib) return;

    auto imports = node->command.GetParams();
    const int importLine = static_cast<int>(tokens[0].GetLine());
    const int importColumn = static_cast<int>(tokens[0].GetColumn());

    auto registerFunc = [&](const BuiltinFunction& bf) {
        FuncMetaData func(
            this->m_entityLogger,
            bf.name,
            bf.returnType,
            bf.params,
            0, 0,
            nullptr,
            true
        );
        func.SetDefined();
        if (!this->m_funcTable.InsertFunction(std::move(func)))
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::NAME_ERROR,
                    "Duplicate import: '" + bf.name + "' was already imported",
                    importLine, importColumn,
                    "Remove the duplicate import"
                )
            );
        }
    };

    auto registerVar = [&](const BuiltinVariable& bv) {
        VarMetaData var(bv.name, bv.type, bv.access, Scope::GLOBAL, 0, true);
        if (!this->m_currentScope->InsertVar(var))
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::NAME_ERROR,
                    "Duplicate import: '" + bv.name + "' was already imported",
                    importLine, importColumn,
                    "Remove the duplicate import"
                )
            );
        }
    };

    if (imports.empty())
    {
        for (const auto& func : lib->functions)
            registerFunc(func);
        for (const auto& var : lib->variables)
            registerVar(var);
        return;
    }

    for (auto import : imports)
    {
        const std::string& symbolName = import.GetTokens()[0].GetValue();
        const int line = static_cast<int>(import.GetTokens()[0].GetLine());
        const int column = static_cast<int>(import.GetTokens()[0].GetColumn());

        const BuiltinFunction* bf = lib->FindFunction(symbolName);
        if (bf)
        {
            registerFunc(*bf);
            continue;
        }

        const BuiltinVariable* bv = lib->FindVariable(symbolName);
        if (bv)
        {
            registerVar(*bv);
            continue;
        }

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Built-in library '" + libName + "' doesn't export: " + symbolName,
                line, column,
                "Check available functions in the '" + libName + "' library"
            )
        );
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

void SemanticAnalysisVisitor::VisitArrayAccess(ASTNode* node)
{
    if (!node || this->m_facade.IsBlockFinished()) return;

    bool nextIsAssignment = node->nodeNextSibling && node->nodeNextSibling->command.GetType() == CommandTypes::ASSIGNMENT;
    const auto& type = node->command.GetType();
    const std::string& name = ASTExtractor::ExtractVarName(node);

    if (!this->m_facade.CheckVarExistence(*this->m_currentScope, name))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Undefined variable '" + name + "' used in expression",
                node->command[IDENTIFIER_NAME_INDEX].GetLine(),
                node->command[IDENTIFIER_NAME_INDEX].GetColumn(),
                "Declare the variable before using it"
            )
        );
    }
    auto varMetaData = this->m_currentScope->FindVar(name).value();

    if (!varMetaData.arraySize.has_value())
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Found array access to '" + name + "' that is a normal variable",
                node->command[IDENTIFIER_NAME_INDEX].GetLine(),
                node->command[IDENTIFIER_NAME_INDEX].GetColumn(),
                "Turn " + name + " into an array"
            )
        );
    }

    if (type == CommandTypes::STRING_ARRAY_ACCESS ||
        type == CommandTypes::STRING_ARRAY_SLICE)
    {
        if (varMetaData.type != Type::STRING_TYPE)
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::NAME_ERROR,
                    "2D array access only allowed for string arrays '",
                    node->command[IDENTIFIER_NAME_INDEX].GetLine(),
                    node->command[IDENTIFIER_NAME_INDEX].GetColumn(),
                    ""
                )
            );
        }
    }

    if (type == CommandTypes::ARRAY_SLICE)
    {
        if (varMetaData.arraySize.has_value())
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::NAME_ERROR,
                    "Array slicing not allowed",
                    node->command[IDENTIFIER_NAME_INDEX].GetLine(),
                    node->command[IDENTIFIER_NAME_INDEX].GetColumn(),
                    ""
                )
            );
        }
    }

    if (nextIsAssignment)
    {
        this->m_lastIdentifierName = name;
    }
    HandleSingleCommandAssignement(node);
}

void SemanticAnalysisVisitor::VisitArrayValue(ASTNode* node)
{
    if (!node || this->m_facade.IsBlockFinished()) return;
    if (!this->m_pendingAssignment || this->m_lastIdentifierName.empty())
    {
        return;
    }

    auto var = this->m_currentScope->FindVar(this->m_lastIdentifierName);
    if (!var.has_value() || var->type != Type::CHAR_TYPE || !var->arraySize.has_value())
    {
        ResetAssignmentState();
        return;
    }

    const auto& params = node->command.GetParams();
    for (size_t i = 0; i < params.size(); ++i)
    {
        ExpressionType elemType = this->m_facade.CheckExpression(
            params[i], this->m_funcTable, *this->m_currentScope);

        if (elemType == ExpressionType::STRING)
        {
            const std::vector<Token>& elemTokens = params[i].GetTokens();
            size_t line = elemTokens.empty() ? this->m_assignmentLine : elemTokens[0].GetLine();
            size_t col = elemTokens.empty() ? this->m_assignmentColumn : elemTokens[0].GetColumn();
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Cannot use string in char array initializer — element " + std::to_string(i + 1) +
                        " must be of type char",
                    line,
                    col,
                    "Use char literals (e.g. 'a') or char expressions, not string literals"
                )
            );
        }
    }

    ResetAssignmentState();
}

void SemanticAnalysisVisitor::VisitIfOrElseIfOrWhile(ASTNode* node)
{
    if (!node || this->m_facade.IsBlockFinished()) return;
    
    // IF/ELSE IF/WHILE ()
    //  condition() <-------- node->nodeFirstChild
    ASTNode* condition = node->nodeFirstChild;
    
    if (!condition) return;

    const auto& tokens = condition->command.GetTokens();
    if (tokens.empty()) return;
    const auto& conditionTokens = tokens[CONDITION_TOKEN_INDEX];

    this->m_facade.SetSuppressDefaultExprCheck(true);

    if (condition->command.GetType() == CommandTypes::EXPRESSION)
    {
        ExpressionType conditionType = EvaluateNodeType(condition);
        if (conditionType != ExpressionType::BOOLEAN)
        { 
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Condition must be a boolean expression",
                    conditionTokens.GetLine(),
                    conditionTokens.GetColumn(),
                    "Provide a boolean expression inside the parentheses"
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
                    "Condition must be boolean, but the given variable is not of type 'bool'",
                    conditionTokens.GetLine(),
                    conditionTokens.GetColumn(),
                    "Provide a boolean expression inside the parentheses"
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
                    "Condition must be boolean, but the function call does not return 'bool'",
                    conditionTokens.GetLine(),
                    conditionTokens.GetColumn(),
                    "Provide a boolean expression inside the parentheses"
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
    this->m_isVarInitialization = false;
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
        case Type::STRING_TYPE:          return "string";
        case Type::CHAR_TYPE:            return "char";
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
        case ExpressionType::STRING:       return "string";
        case ExpressionType::CHAR:         return "char";
        case ExpressionType::VOID_TYPE:    return "void";
        default:                           return "unknown";
    }
}

ExpressionType SemanticAnalysisVisitor::EvaluateNodeType(ASTNode* node) noexcept
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

        if (var.has_value() && !var->arraySize.has_value()) 
        {
            return this->m_facade.TypeToExpressionType(var->type);
        }
        else
        {
            return ExpressionType::UNKNOWN;
        }
    }
    
    else if (nodeType == CommandTypes::EXPRESSION)
    {
        return this->m_facade.CheckExpression(
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

        return this->m_facade.TypeToExpressionType(func->GetRType());
    }

    else if (nodeType == CommandTypes::SIZEOF)
    {
        return ExpressionType::MATHEMATICAL; // sizeof always returns an int
    }

    else if (nodeType == CommandTypes::ARRAY_ACCESS ||
             nodeType == CommandTypes::ARRAY_SLICE ||
             nodeType == CommandTypes::STRING_ARRAY_ACCESS ||
             nodeType == CommandTypes::STRING_ARRAY_SLICE)
    {
        return this->m_facade.CheckExpression(
            node->command,
            this->m_funcTable,
            *this->m_currentScope
        );
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
    if (!this->m_facade.EnteredBlock(blockkind, isVoidReturnType))
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

void SemanticAnalysisVisitor::HandleExitedBlockCheck(ASTNode* node)
{
    if (!this->m_facade.ExitedBlock())
    {
        const auto& tokens = node->command.GetTokens();
        if (tokens.empty()) return;
        const auto& errorTokens = tokens[FIRST_TOKEN_INDEX];

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::FUNC_MISSING_RETURN_ERROR,

                "Not all code paths in non-void function '" + 
                this->m_currentFunctionName + 
                "' return a value",

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

    ExpressionType expressionType = this->m_facade.CheckExpression(
        node->command,
        this->m_funcTable,
        *this->m_currentScope
    );

    if (expressionType == ExpressionType::UNKNOWN)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::TYPE_ERROR,
                "Cannot evaluate expression — possible type mismatch or use of an undefined variable/function",
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
        !this->m_facade.CheckAssignment(
            this->m_lastIdentifierName,
            expressionType,
            *this->m_currentScope,
            this->m_isVarInitialization)
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
