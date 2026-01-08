#include "includes/semantic_analysis/semantic_analysis_visitor.h"

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
    m_identifierError(false)
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
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Check if variable already declared, remember no shadowing allowed!"
            )
        );
    }

    VarMetaData var = this->m_varFactory.CreateVarMetadata(
        varName, varType, varAccessRight, varScope
    );

    if (!this->m_currentScope->InsertVar(var))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::INTERNAL_ERROR,
                "Failed to insert variable '" + varName + "'",
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "This is an internal error, please report it"
            )
        );
    }
}

void SemanticAnalysisVisitor::VisitIdentifier(ASTNode* node)
{
    if (!node)
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
    if (!node) return;
    
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
    if (!node)
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
    this->m_assignmentLine = tokens[0].GetLine();
    this->m_assignmentColumn = tokens[0].GetColumn();
}

void SemanticAnalysisVisitor::VisitExpression(ASTNode* node)
{
    if (!node)
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
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
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

void SemanticAnalysisVisitor::VisitFuncDeclaration(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[0].GetValue();
    
    if (SemanticCheckerFacade::CheckFuncDeclaration(this->m_funcTable, funcName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Function '" + funcName + "' is already declared",
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Function names must be unique"
            )
        );
    }
    
    this->m_currentFunctionName = funcName;
}

void SemanticAnalysisVisitor::VisitFuncDefinition(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[0].GetValue();
    
    if (SemanticCheckerFacade::CheckFuncDefinition(this->m_funcTable, funcName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Function '" + funcName + "' is already defined",
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Function can only be defined once"
            )
        );
    }
    
    this->HandleEnteredBlockCheck(node, SemanticCheckerFacade::BlockKind::Function);
    this->HandleExitedBlockCheck(node);
}

void SemanticAnalysisVisitor::VisitFuncCall(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[0].GetValue();
    
    if (!SemanticCheckerFacade::CheckFuncDeclaration(this->m_funcTable, funcName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Undefined function '" + funcName + "'",
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Function must be declared before calling"
            )
        );
        return;
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
                tokens[0].GetLine(),
                tokens[0].GetColumn(),
                "Check the number and types of arguments"
            )
        );
    }
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
    if (!node) return;
    
    this->HandleEnteredBlockCheck(node, SemanticCheckerFacade::BlockKind::Else);
    this->HandleExitedBlockCheck(node);
}

void SemanticAnalysisVisitor::VisitWhile(ASTNode* node)
{
    this->VisitIfOrElseIfOrWhile(node);
}

void SemanticAnalysisVisitor::VisitFor(ASTNode* node)
{
    if (!node) return;
    
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
    
    if (conditionNode)
    {
        ExpressionType condType = EvaluateNodeType(conditionNode);
        
        if (condType != ExpressionType::BOOLEAN)
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "for loop condition must be a boolean expression",
                    tokens[0].GetLine(),
                    tokens[0].GetColumn(),
                    "Provide a boolean condition"
                )
            );
        }
    }
    
    this->HandleEnteredBlockCheck(node, SemanticCheckerFacade::BlockKind::Loop);
}

void SemanticAnalysisVisitor::VisitReturn(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    size_t line = tokens.empty() ? 0 : tokens[0].GetLine();
    size_t column = tokens.empty() ? 0 : tokens[0].GetColumn();
    
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

    if (
        !SemanticCheckerFacade::CheckFunctionReturnType(
            this->EvaluateNodeType(node->nodeNextSibling),
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
    if (!node) return;
    
    if (!SemanticCheckerFacade::CheckControlStatement())
    {    
        const std::string& statement = tokens[0].GetValue();
        size_t line = tokens[0].GetLine();
        size_t column = tokens[0].GetColumn();

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
    this->m_currentScope = this->m_currentScope->CreateBlock();
}

void SemanticAnalysisVisitor::VisitBlockEnd(ASTNode* node)
{
    if (!node) return;
    this->HandleExitedBlockCheck(node);
}

void SemanticAnalysisVisitor::VisitEndOfCommand(ASTNode* node)
{
    // skip, no logic needed
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
    if (!node) return;
    
    // IF/ELSE IF/WHILE ()
    //  condition() <-------- node->nodeFirstChild
    ASTNode* condition = node->nodeFirstChild;
    
    if (!condition) return;

    const auto& conditionTokens = condition->command.GetTokens()[0];

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
        const std::string& varName = condition->command.GetTokens()[0].GetValue();
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
        const auto func = this->m_funcTable.FindFunctionByName(this->m_currentFunctionName);

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
            "Invalid condition at line: " + conditionTokens.GetLine()
        );
    }

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

    this->HandleEnteredBlockCheck(node, blockKind);
    
    if (!SemanticCheckerFacade::ExitedBlock())
    {
        const auto& errorToken = node->command.GetTokens()[0];

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::FUNC_MISSING_RETURN_ERROR,
                "Not all paths of non void function have a return statement",
                errorToken.GetLine(),
                errorToken.GetLine(),
                "Non void functions must return a value at all end-points"
            )
        );
    }
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
        case Type::UNSIGNED_DOUBLE_TYPE: return "unsigned double";
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
    
    // base case: identifier -> lookup variable type
    if (nodeType == CommandTypes::IDENTIFIER_VALUE)
    {
        const std::string& varName = node->command.GetTokens()[0].GetValue();
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
    
    // base case: expression -? CheckExpression handles its internal params
    if (nodeType == CommandTypes::EXPRESSION)
    {
        return SemanticCheckerFacade::CheckExpression(
            node->command, 
            this->m_funcTable, 
            *this->m_currentScope
        );
    }
    
    // otherwise, recursivly get function call
    if (nodeType == CommandTypes::FINISHED_FUNC_CALL)
    {        
        FuncMetaData* funcData = m_funcTable.FindFunctionByName(this->m_currentFunctionName);
        if (!funcData) return ExpressionType::UNKNOWN;

        // return the function's return type
        return SemanticCheckerFacade::TypeToExpressionType(funcData->GetRType());
    }
    
    return ExpressionType::UNKNOWN;
}

void SemanticAnalysisVisitor::HandleEnteredBlockCheck(
        ASTNode* node, 
        SemanticCheckerFacade::BlockKind blockkind
    ) const
{
    const auto& func = this->m_funcTable.FindFunctionByName(this->m_currentFunctionName);

    if (!func) return;

    bool isVoidReturnType = func->GetRType() == Type::VOID_TYPE;
    if (
        !SemanticCheckerFacade::EnteredBlock(
            SemanticCheckerFacade::BlockKind::Function, 
            isVoidReturnType
        )
    )
    {
        const auto& errorTokens = node->command.GetTokens()[0];

        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::STATEMENT_OUT_OF_BLOCK_ERROR,
                "Invalid usage of else if/else keywords without matching if keyword",
                errorTokens.GetLine(),
                errorTokens.GetColumn(),
                "else if/else keyword are only allowed after usage of if keyword"
            )
        );
    }
}

void SemanticAnalysisVisitor::HandleExitedBlockCheck(ASTNode* node) const
{
    if (!SemanticCheckerFacade::ExitedBlock())
    {
        const auto& errorTokens = node->command.GetTokens()[0];

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
