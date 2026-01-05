#include "includes/semantic_analysis/semantic_analysis_visitor.h"
#include "semantic_analysis_visitor.h"

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
                tokens[IDENTIFIER_NAME_INDEX].GetLine();
                tokens[IDENTIFIER_NAME_INDEX].GetColumn();
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
    size_t line = tokens[0].GetLine();
    size_t column = tokens[0].GetColumn();
    
    if (SemanticCheckerFacade::CheckFuncDefinition(this->m_funcTable, funcName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Function '" + funcName + "' is already defined",
                line,
                static_cast<uint16_t>(column),
                "Function can only be defined once"
            )
        );
    }
    
    this->m_currentFunctionName = funcName;
    
    FuncMetaData* funcData = this->m_funcTable.FindFunctionByName(funcName);
    bool isVoid = funcData && funcData->GetRType() == Type::VOID_TYPE;
    SemanticCheckerFacade::EnteredBlock(SemanticCheckerFacade::BlockKind::Function, isVoid);
}

void SemanticAnalysisVisitor::VisitFuncCall(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& funcName = tokens[0].GetValue();
    size_t line = tokens[0].GetLine();
    size_t column = tokens[0].GetColumn();
    
    if (!SemanticCheckerFacade::CheckFuncDeclaration(this->m_funcTable, funcName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Undefined function '" + funcName + "'",
                line,
                static_cast<uint16_t>(column),
                "Function must be declared before calling"
            )
        );
        return;
    }
    
    if (!this->m_funcCallArgTypes.empty())
    {
        if (!SemanticCheckerFacade::CheckFuncArguments(
                this->m_funcCallArgTypes, 
                funcName, 
                this->m_funcTable))
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::ARGUMENT_ERROR,
                    "Invalid arguments for function '" + funcName + "'",
                    line,
                    static_cast<uint16_t>(column),
                    "Check the number and types of arguments"
                )
            );
        }
        this->m_funcCallArgTypes.clear();
    }
}

void SemanticAnalysisVisitor::VisitIf(ASTNode* node)
{
    if (!node) return;
    
    if (this->m_lastExpressionType != ExpressionType::BOOLEAN && 
        this->m_lastExpressionType != ExpressionType::UNKNOWN)
    {
        const std::vector<Token>& tokens = node->command.GetTokens();
        if (!tokens.empty())
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "If condition must be a boolean expression",
                    tokens[0].GetLine(),
                    static_cast<uint16_t>(tokens[0].GetColumn()),
                    "Use a boolean expression or comparison"
                )
            );
        }
    }
    
    SemanticCheckerFacade::EnteredBlock(SemanticCheckerFacade::BlockKind::If, false);
}

void SemanticAnalysisVisitor::VisitElif(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    
    if (!SemanticCheckerFacade::EnteredBlock(SemanticCheckerFacade::BlockKind::Elif, false))
    {
        if (!tokens.empty())
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::SYNTAX_ERROR,
                    "Elif must follow an if or elif block",
                    tokens[0].GetLine(),
                    static_cast<uint16_t>(tokens[0].GetColumn()),
                    "Check block structure"
                )
            );
        }
    }
    
    if (this->m_lastExpressionType != ExpressionType::BOOLEAN && 
        this->m_lastExpressionType != ExpressionType::UNKNOWN)
    {
        if (!tokens.empty())
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Elif condition must be a boolean expression",
                    tokens[0].GetLine(),
                    static_cast<uint16_t>(tokens[0].GetColumn()),
                    "Use a boolean expression or comparison"
                )
            );
        }
    }
}

void SemanticAnalysisVisitor::VisitElse(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    
    if (!SemanticCheckerFacade::EnteredBlock(SemanticCheckerFacade::BlockKind::Else, false))
    {
        if (!tokens.empty())
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::SYNTAX_ERROR,
                    "Else must follow an if or elif block",
                    tokens[0].GetLine(),
                    static_cast<uint16_t>(tokens[0].GetColumn()),
                    "Check block structure"
                )
            );
        }
    }
}

void SemanticAnalysisVisitor::VisitWhile(ASTNode* node)
{
    if (!node) return;
    
    if (this->m_lastExpressionType != ExpressionType::BOOLEAN && 
        this->m_lastExpressionType != ExpressionType::UNKNOWN)
    {
        const std::vector<Token>& tokens = node->command.GetTokens();
        if (!tokens.empty())
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "While condition must be a boolean expression",
                    tokens[0].GetLine(),
                    static_cast<uint16_t>(tokens[0].GetColumn()),
                    "Use a boolean expression or comparison"
                )
            );
        }
    }
    
    SemanticCheckerFacade::EnteredBlock(SemanticCheckerFacade::BlockKind::Loop, false);
}

void SemanticAnalysisVisitor::VisitFor(ASTNode* node)
{
    if (!node) return;
    
    SemanticCheckerFacade::EnteredBlock(SemanticCheckerFacade::BlockKind::Loop, false);
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
                "Return statement outside of function",
                line,
                static_cast<uint16_t>(column),
                "Return can only be used inside a function body"
            )
        );
        return;
    }
    
    if (SemanticCheckerFacade::IsBlockFinished())
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::SYNTAX_ERROR,
                "Unreachable code after return",
                line,
                static_cast<uint16_t>(column),
                "Remove unreachable code"
            )
        );
    }
    
    if (!this->m_currentFunctionName.empty())
    {
        if (!SemanticCheckerFacade::CheckFunctionReturnType(
                this->m_lastExpressionType,
                this->m_currentFunctionName,
                this->m_funcTable))
        {
            this->m_errorManager.StoreError(
                CompileTimeError(
                    ErrorTypes::TYPE_ERROR,
                    "Return type mismatch in function '" + this->m_currentFunctionName + "'",
                    line,
                    static_cast<uint16_t>(column),
                    "Ensure return type matches function declaration"
                )
            );
        }
    }
}

void SemanticAnalysisVisitor::VisitControlStatement(ASTNode* node)
{
    if (!node) return;
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    if (tokens.empty()) return;
    
    const std::string& statement = tokens[0].GetValue();
    size_t line = tokens[0].GetLine();
    size_t column = tokens[0].GetColumn();
    
    if (!SemanticCheckerFacade::CheckControlStatement())
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::STATEMENT_OUT_OF_BLOCK_ERROR,
                "'" + statement + "' statement outside of loop",
                line,
                static_cast<uint16_t>(column),
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
    
    const std::vector<Token>& tokens = node->command.GetTokens();
    size_t line = tokens.empty() ? 0 : tokens[0].GetLine();
    size_t column = tokens.empty() ? 0 : tokens[0].GetColumn();
    
    if (!SemanticCheckerFacade::ExitedBlock())
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::FUNC_MISSING_RETURN_ERROR,
                "Not all code paths return a value in function '" + this->m_currentFunctionName + "'",
                line,
                static_cast<uint16_t>(column),
                "Ensure all code paths return a value"
            )
        );
    }
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

void SemanticAnalysisVisitor::SignalFailureForNextHandlers()
{
    this->m_identifierError = true;
    this->m_lastIdentifierName.clear();
}

void SemanticAnalysisVisitor::ResetAssignmentState()
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