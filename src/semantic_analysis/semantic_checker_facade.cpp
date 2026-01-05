#include "includes/semantic_analysis/semantic_checker_facade.h"

bool SemanticCheckerFacade::m_bLoopFlag = false;
size_t SemanticCheckerFacade::m_uLoopBlockCnt = 0;
std::stack<SemanticCheckerFacade::OperationKind> SemanticCheckerFacade::m_operationHistory;
std::stack<SemanticCheckerFacade::BlockKind> SemanticCheckerFacade::m_entryHistory;
bool SemanticCheckerFacade::m_bIsReturnVoid = false;
bool SemanticCheckerFacade::m_bFinishedBlock = false;

bool SemanticCheckerFacade::CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& varName)
{
    DefaultExpressionChecks();
    return currVarTableNode.FindVar(varName) != std::nullopt;
}

bool SemanticCheckerFacade::CheckFuncDeclaration(const FuncTable& funcTable, const std::string& funcName)
{
    DefaultExpressionChecks();
    FuncMetaData* searchedFuncMetadata = funcTable.FindFunctionByName(funcName);
    if (!searchedFuncMetadata) return false;
    return searchedFuncMetadata->IsFuncDeclared();
}

bool SemanticCheckerFacade::CheckFuncDefinition(const FuncTable& funcTable, const std::string& funcName)
{
    DefaultExpressionChecks();
    FuncMetaData* searchedFuncMetadata = funcTable.FindFunctionByName(funcName);
    if (!searchedFuncMetadata) return false;
    return searchedFuncMetadata->IsFuncDefined();
}

ExpressionType SemanticCheckerFacade::CheckExpression(
    const Command& expression,
    const FuncTable& funcTable,
    const VarTableNode& currVarTableNode)
{
    DefaultExpressionChecks();
    /// @note: The semantic checker doesnt distuinguish between INT and DOUBLE only between MATH and BOOL, That's why it only uses DOUBLE_TYPE
    ExpressionType expressionType = ExpressionType::UNKNOWN;
    size_t uParamIndex = 0;

    // Incase the expression has a single void param.
    if (expression.GetTokens().size() == 1 && expression[FIRST_TOKEN_INDEX].GetValue() == "void")
    {
        return ExpressionType::VOID_TYPE;
    }

    for (size_t i = 0; i < expression.GetTokens().size(); i++)
    {
        // If its a parameter its either identifier function or sizeof
        if (expression[i].GetType() == TokenTypes::HAS_PARAM)
        {
            ExpressionType paramType = ExpressionType::UNKNOWN;
            const Command& currCommand = expression.GetParams()[uParamIndex];

            // Firstly we find the paramType.
            if (currCommand.GetType() == CommandTypes::IDENTIFIER_VALUE)
            {
                auto doesExist = currVarTableNode.FindVar(currCommand[IDENTIFIER_INDEX].GetValue());

                if (doesExist) paramType = TypeToExpressionType(*varData.type); // Saving the var's type.
                else return ExpressionType::UNKNOWN; // Incase the var doesnt exist.
            }
            else if (currCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
            {
                FuncMetaData* funcData = funcTable.FindFunctionByName(currCommand[IDENTIFIER_INDEX].GetValue());

                if (funcData) paramType = TypeToExpressionType(funcData->GetRType()); // Saving the func's return type.
                else return ExpressionType::UNKNOWN; //Incase the func doesnt exist
            }
            if (currCommand.GetType() == CommandTypes::SIZEOF)
            {
                paramType = ExpressionType::MATHEMATICAL; // Sizeof return int.
            }
            else return ExpressionType::UNKNOWN; // Undefined behaviour.
            
            // Checking that the paramType is corrent.
            if (expressionType == ExpressionType::UNKNOWN) // Incase it's first expression part.
            {
                expressionType = paramType; // Setting the paramType as the expressionType because it's the first expression part.
            }
            else if (paramType != expressionType) // If the expression isn't the same type all the way it's an error.
            {
                return ExpressionType::UNKNOWN;
            }
            uParamIndex++;
        }
        else if (expression[i].GetValue() == "void") // Void in the middle of the expression is illegal.
        {
            return ExpressionType::UNKNOWN;
        }
        else // Any const value or operator
        {
            if (expressionType == ExpressionType::UNKNOWN) // Incase it's first expression part.
            {
                expressionType = TokenToExpressionType(expression[i]); // Setting the paramType as the first token type because it's the first expression part.
            }
            else if (TokenToExpressionType(expression[i]) != expressionType) // If the expression isn't the same type all the way it's an error.
            {
                return ExpressionType::UNKNOWN;
            }
        }
    }

    return expressionType;
}

bool SemanticCheckerFacade::CheckFuncArguments(const std::vector<ExpressionType>& paramTypes, const std::string& sFuncName, const FuncTable& funcTable)
{
    DefaultExpressionChecks();

    const FuncMetaData* func = funcTable.FindFunctionByName(sFuncName);
    if (!func) return false;
    const std::vector<VarMetaData>& funcParams = func->GetFuncParams();

    // Checking that the declaration and given parameters are the same amount.
    if (funcParams.size() != paramTypes.size()) return false;

    if (funcParams[FIRST_PARAMETER_INDEX].type == Type::VOID_TYPE) return true; /// Void as a param is a hardcoded check inside the parser.

    // Checking that all of the parameter types are the right function declared types.
    for (size_t i = 0; i < paramTypes.size(); i++)
    {
        if (!CompareTypes(funcParams[i].type, paramTypes[i])) return false;
    }
    return true;
}

inline bool SemanticCheckerFacade::CheckReturnInsideFunc()
{
    return !this->m_operationHistory.empty();
}

bool SemanticCheckerFacade::CheckFunctionReturnType(const ExpressionType& returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable)
{
    if (!CheckReturnInsideFunc()) return false;
    if (!this->m_bFinishedBlock)
    {
        SwitchTopOfOperationHistory(OperationKind::Return);
        this->m_bFinishedBlock = true;
    }
    const FuncMetaData* func = funcTable.FindFunctionByName(sFuncName);
    if (!func) return false;
    return CompareTypes(func->GetRType(), returnExpressionType);
}

bool SemanticCheckerFacade::CheckAssignment(const std::string& sAssignedVarName, const ExpressionType& assigmentExpressionType, const VarTableNode& currVarTableNode)
{
    DefaultExpressionChecks();
    VarMetaData varData = *currVarTableNode.FindVar(sAssignedVarName);
    if (varData.accessRights == AccessRights::CONST_ACCESS || varData.accessRights == AccessRights::READ_ONLY_ACCESS) return false;
    return CompareTypes(varData.type, assigmentExpressionType);
}

bool SemanticCheckerFacade::CheckControlStatement()
{
    return !this->m_entryHistory.empty() && this->m_entryHistory.top() == BlockKind::Loop;
}

bool SemanticCheckerFacade::EnteredBlock(const BlockKind& entryKind, bool bIsReturnVoid)
{
    if (this->m_bLoopFlag)
    {
        this->m_uLoopBlockCnt++;
    }
    else
    {
        switch (entryKind)
        {
        case BlockKind::Function:
            // Resetting for a new func.
            this->m_operationHistory = std::stack<OperationKind>();
            this->m_entryHistory = std::stack<BlockKind>();
            this->m_uLoopBlockCnt = 0;
            this->m_operationHistory.push(OperationKind::None);
            this->m_bIsReturnVoid = bIsReturnVoid;
            this->m_bLoopFlag = false;
            this->m_bFinishedBlock = false;
            break;
        
        case BlockKind::If:
            this->m_operationHistory.push(OperationKind::None); // Starting a new block.
            break;
        
        case BlockKind::Else:
            if (ElseElifBlockEnter()) return false;
            this->m_operationHistory.push(OperationKind::None); // Starting a new block.
            break;
        
        case BlockKind::Elif:
            if (ElseElifBlockEnter()) return false;
            this->m_operationHistory.push(OperationKind::None); // Starting a new block.
            break;

        case BlockKind::Loop:
            this->m_bLoopFlag = true;
            ++this->m_uLoopBlockCnt;
            break;

        default:
            break;
        }
        this->m_entryHistory.push(entryKind);
    }

    return true;
}

bool SemanticCheckerFacade::ExitedBlock()
{
    if (this->m_bLoopFlag)
    {
        this->m_uLoopBlockCnt--;
        if (this->m_uLoopBlockCnt == 0) this->m_bLoopFlag = false;
        return true;
    }
    OperationKind lastOperation = this->m_operationHistory.top();

    switch (this->m_entryHistory.top())
    {
    case BlockKind::Function:
        if (!this->m_bIsReturnVoid)
        {
            switch (this->m_operationHistory.top())
            {
            // The function stopped with a return.
            case OperationKind::ElseReturn:
                break;
            case OperationKind::Return:
                break;
            // The function stopped without a return.
            default:
                return false; // non void func didnt return.
                break;
            }
            this->m_bFinishedBlock = false;
            this->m_bIsReturnVoid = false;
        }
        break;
    
    case BlockKind::If:
        this->m_operationHistory.pop();
        // If The block ended with a return.
        if (lastOperation == OperationKind::Return ||
            lastOperation == OperationKind::ElseReturn)
        {
            SwitchTopOfOperationHistory(OperationKind::IfReturn);
            this->m_bFinishedBlock = false;
        }
        else
        {
            SwitchTopOfOperationHistory(OperationKind::If);
            this->m_bFinishedBlock = false;
        }
        break;

    case BlockKind::Else:
        ElseElifBlockExit(OperationKind::Else);
        break;

    case BlockKind::Elif:
        ElseElifBlockExit(OperationKind::Elif);
        break;

    default:
        break;
    }

    this->m_entryHistory.pop();
    return true;
}

bool SemanticCheckerFacade::IsBlockFinished()
{
    return !this->m_operationHistory.empty() &&
        (this->m_operationHistory.top() == OperationKind::Return ||
        this->m_operationHistory.top() == OperationKind::ElseReturn);
}

ExpressionType SemanticCheckerFacade::TypeToExpressionType(const Type& type)
{
    if (type == Type::BOOL_TYPE) return ExpressionType::BOOLEAN;
    else if (type == Type::VOID_TYPE) return ExpressionType::VOID_TYPE;
    else return ExpressionType::MATHEMATICAL;
}

ExpressionType SemanticCheckerFacade::TokenToExpressionType(const Token& token)
{
    /// First we check if the token is an operator
    if (
        (token.GetValue() != "=" && token.GetType() == TokenTypes::OPERATOR)
        ||
        token.GetType() == TokenTypes::KEYWORD)
    {
        if (token.GetValue() == "==" ||
            token.GetValue() == "and" ||
            token.GetValue() == "or" ||
            token.GetValue() == "not" ||
            token.GetValue() == "xor" ||
            token.GetValue() == "is")
        {
            return ExpressionType::BOOLEAN;
        }
        else return ExpressionType::MATHEMATICAL;
    }
    /// If it's a value
    else if (token.GetValue() == "true" || token.GetValue() == "false")
    {
        return ExpressionType::BOOLEAN;
    }
    else return ExpressionType::MATHEMATICAL;
}

bool SemanticCheckerFacade::CompareTypes(const Type& expectedType, const ExpressionType& gottenType)
{
    if (gottenType == ExpressionType::UNKNOWN) return false;
    if (expectedType == Type::BOOL_TYPE && gottenType == ExpressionType::BOOLEAN) return true;
    if (expectedType == Type::VOID_TYPE && gottenType == ExpressionType::VOID_TYPE) return true;
    if (gottenType == ExpressionType::MATHEMATICAL &&
        (expectedType == Type::DOUBLE_TYPE ||
        expectedType == Type::INT_TYPE ||
        expectedType == Type::UNSIGNED_DOUBLE_TYPE ||
        expectedType == Type::UNSIGNED_INT_TYPE)
    ) return true;

    return false;
}

inline void SemanticCheckerFacade::DefaultExpressionChecks()
{
    if (!this->m_bLoopFlag &&
        !this->m_bFinishedBlock)
    {
        SwitchTopOfOperationHistory(OperationKind::Expression);
    }
}

bool SemanticCheckerFacade::ElseElifBlockEnter()
{
    if (this->m_operationHistory.top() != OperationKind::If &&
        this->m_operationHistory.top() != OperationKind::IfReturn &&
        this->m_operationHistory.top() != OperationKind::Elif &&
        this->m_operationHistory.top() != OperationKind::ElifReturn)
    {
        return true; // else not after if or elif
    }
    this->m_operationHistory.push(OperationKind::None);
    return false;
}

void SemanticCheckerFacade::ElseElifBlockExit(OperationKind kind)
{
    OperationKind lastOperation = this->m_operationHistory.top();
    this->m_operationHistory.pop();
    // If The block ended with a return and before that the last condition also returned.
    if (
        (lastOperation == OperationKind::Return ||
        lastOperation == OperationKind::ElseReturn)
        &&
        (this->m_operationHistory.top() == OperationKind::IfReturn ||
        this->m_operationHistory.top() == OperationKind::ElifReturn)
    )
    {
        switch (kind)
        {
        case OperationKind::Elif:
            SwitchTopOfOperationHistory(OperationKind::ElifReturn);
            this->m_bFinishedBlock = false;
            break;
        
        case OperationKind::Else:
            SwitchTopOfOperationHistory(OperationKind::ElseReturn);
            this->m_bFinishedBlock = true;
            break;
        default:
            break;
        }
    }
    else
    {
        this->m_bFinishedBlock = false;
        SwitchTopOfOperationHistory(kind);
    }
}

inline void SemanticCheckerFacade::SwitchTopOfOperationHistory(OperationKind operationKind)
{
    if (!this->m_operationHistory.empty())
    {
        this->m_operationHistory.pop();
        this->m_operationHistory.push(operationKind);
    }
    else
    {
        this->m_operationHistory.push(operationKind);
    }
}
