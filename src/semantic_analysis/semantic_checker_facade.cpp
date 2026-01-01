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
    if (funcTable.FindFunctionByName(funcName) == nullptr) return false;
    return funcTable.FindFunctionByName(funcName)->IsFuncDeclared();
}

bool SemanticCheckerFacade::CheckFuncDefinition(const FuncTable& funcTable, const std::string& funcName)
{
    DefaultExpressionChecks();
    if (funcTable.FindFunctionByName(funcName) == nullptr) return false;
    return funcTable.FindFunctionByName(funcName)->IsFuncDefined();
}

ExpressionType SemanticCheckerFacade::CheckExpression(
    const Command& expression,
    const FuncTable& funcTable,
    const VarTableNode& currVarTableNode)
{
    DefaultExpressionChecks();
    /// @note: The semantic checker doesnt distuinguish between INT and DOUBLE only between MATH and BOOL, That's why it only uses DOUBLE_TYPE
    ExpressionType currType = ExpressionType::UNKNOWN;
    size_t uParamIndex = 0;

    // Incase the expression has a single void param
    if (expression.GetTokens().size() == 1 && expression[0].GetValue() == "void")
    {
        return ExpressionType::VOID_TYPE;
    }

    for (size_t i = 0; i < expression.GetTokens().size(); i++)
    {
        // If its a parameter its either identifier or function.
        if (expression[i].GetType() == TokenTypes::HAS_PARAM)
        {
            ExpressionType paramType = ExpressionType::UNKNOWN;
            const Command& currCommand = expression.GetParams()[uParamIndex];

            // Identifier.
            if (currCommand.GetType() == CommandTypes::IDENTIFIER_VALUE)
            {
                auto doesExist = currVarTableNode.FindVar(currCommand[0].GetValue());
                if (doesExist)
                {
                    auto varData = *doesExist;
                    paramType = TypeToExpressionType(varData.type);
                }
                else return ExpressionType::UNKNOWN;
            }
            // Function.
            else if (currCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
            {
                FuncMetaData* funcData = funcTable.FindFunctionByName(currCommand[0].GetValue());
                if (funcData) paramType = TypeToExpressionType(funcData->GetRType());
                else return ExpressionType::UNKNOWN;
            }
            else return ExpressionType::UNKNOWN;
            
            if (currType == ExpressionType::UNKNOWN) // Incase it's first expression part.
            {
                currType = paramType;
            }
            else if (paramType != currType)
            {
                return ExpressionType::UNKNOWN;
            }
            uParamIndex++;
        }
        else if (expression[i].GetValue() == "void")
        {
            return ExpressionType::UNKNOWN;
        }
        else
        {
            if (currType == ExpressionType::UNKNOWN) // Incase it's first expression part.
            {
                currType = TokenToExpressionType(expression[i]);
            }
            else if (TokenToExpressionType(expression[i]) != currType)
            {
                return ExpressionType::UNKNOWN;
            }
        }
    }

    return currType;
}

bool SemanticCheckerFacade::CheckFuncArguments(const std::vector<ExpressionType>& paramTypes, const std::string& sFuncName, const FuncTable& funcTable)
{
    DefaultExpressionChecks();

    const FuncMetaData* func = funcTable.FindFunctionByName(sFuncName);
    if (!func) return false;
    const std::vector<VarMetaData>& funcParams = func->GetFuncParams();

    // Checking that the declaration and given parameters are the same amount.
    if (funcParams.size() != paramTypes.size()) return false;

    if (funcParams[0].type == Type::VOID_TYPE) return true; /// Void as a param is a hardcoded check inside the parser.

    // Checking that all of the parameter types are the right function declared types.
    for (size_t i = 0; i < paramTypes.size(); i++)
    {
        if (!CompareTypes(funcParams[i].type, paramTypes[i])) return false;
    }
    return true;
}

inline bool SemanticCheckerFacade::CheckReturnInsideFunc()
{
    return !m_operationHistory.empty();
}

bool SemanticCheckerFacade::CheckFunctionReturnType(const ExpressionType& returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable)
{
    if (!CheckReturnInsideFunc()) return false;
    if (!m_bFinishedBlock)
    {
        SwitchTop(OperationKind::Return);
        m_bFinishedBlock = true;
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
    return !m_entryHistory.empty() && m_entryHistory.top() == BlockKind::Loop;
}

bool SemanticCheckerFacade::EnteredBlock(const BlockKind& entryKind, bool bIsReturnVoid)
{
    if (m_bLoopFlag)
    {
        m_uLoopBlockCnt++;
    }
    else
    {
        switch (entryKind)
        {
        case BlockKind::Function:
            // Resetting for a new func.
            m_operationHistory = std::stack<OperationKind>();
            m_entryHistory = std::stack<BlockKind>();
            m_uLoopBlockCnt = 0;
            m_operationHistory.push(OperationKind::None);
            m_bIsReturnVoid = bIsReturnVoid;
            m_bLoopFlag = false;
            m_bFinishedBlock = false;
            break;
        
        case BlockKind::If:
            m_operationHistory.push(OperationKind::None); // Starting a new block.
            break;
        
        case BlockKind::Else:
            if (ElseElifBlockEnter()) return false;
            m_operationHistory.push(OperationKind::None); // Starting a new block.
            break;
        
        case BlockKind::Elif:
            if (ElseElifBlockEnter()) return false;
            m_operationHistory.push(OperationKind::None); // Starting a new block.
            break;

        case BlockKind::Loop:
            m_bLoopFlag = true;
            ++m_uLoopBlockCnt;
            break;

        default:
            break;
        }
        m_entryHistory.push(entryKind);
    }

    return true;
}

bool SemanticCheckerFacade::ExitedBlock()
{
    if (m_bLoopFlag)
    {
        m_uLoopBlockCnt--;
        if (m_uLoopBlockCnt == 0) m_bLoopFlag = false;
        return true;
    }
    OperationKind lastOperation = m_operationHistory.top();

    switch (m_entryHistory.top())
    {
    case BlockKind::Function:
        if (!m_bIsReturnVoid)
        {
            switch (m_operationHistory.top())
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
        }
        break;
    
    case BlockKind::If:
        m_operationHistory.pop();
        // If The block ended with a return.
        if (lastOperation == OperationKind::Return ||
            lastOperation == OperationKind::ElseReturn)
        {
            SwitchTop(OperationKind::IfReturn);
            m_bFinishedBlock = false;
        }
        else
        {
            SwitchTop(OperationKind::If);
            m_bFinishedBlock = false;
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

    m_entryHistory.pop();
    return true;
}

bool SemanticCheckerFacade::IsBlockFinished()
{
    return !m_operationHistory.empty() &&
        (m_operationHistory.top() == OperationKind::Return ||
        m_operationHistory.top() == OperationKind::ElseReturn);
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
    if (token.GetValue() != "=" && token.GetType() == TokenTypes::OPERATOR || token.GetType() == TokenTypes::KEYWORD)
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
    if (gottenType == ExpressionType::MATHEMATICAL && (
        expectedType == Type::DOUBLE_TYPE ||
        expectedType == Type::INT_TYPE ||
        expectedType == Type::UNSIGNED_DOUBLE_TYPE ||
        expectedType == Type::UNSIGNED_INT_TYPE)) return true;

    return false;
}

inline void SemanticCheckerFacade::DefaultExpressionChecks()
{
    if (!m_bLoopFlag &&
        !m_bFinishedBlock)
    {
        SwitchTop(OperationKind::Expression);
    }
}

bool SemanticCheckerFacade::ElseElifBlockEnter()
{
    if (m_operationHistory.top() != OperationKind::If &&
        m_operationHistory.top() != OperationKind::IfReturn &&
        m_operationHistory.top() != OperationKind::Elif &&
        m_operationHistory.top() != OperationKind::ElifReturn)
    {
        return true; // else not after if or elif
    }
    m_operationHistory.push(OperationKind::None);
    return false;
}

void SemanticCheckerFacade::ElseElifBlockExit(OperationKind kind)
{
    OperationKind lastOperation = m_operationHistory.top();
    m_operationHistory.pop();
    // If The block ended with a return and before that the last condition also returned.
    if ((lastOperation == OperationKind::Return ||
        lastOperation == OperationKind::ElseReturn) &&
        (m_operationHistory.top() == OperationKind::IfReturn ||
        m_operationHistory.top() == OperationKind::ElifReturn))
    {
        switch (kind)
        {
        case OperationKind::Elif:
            SwitchTop(OperationKind::ElifReturn);
            m_bFinishedBlock = false;
            break;
        
        case OperationKind::Else:
            SwitchTop(OperationKind::ElseReturn);
            m_bFinishedBlock = true;
            break;
        default:
            break;
        }
    }
    else
    {
        m_bFinishedBlock = false;
        SwitchTop(kind);
    }
}

void SemanticCheckerFacade::SwitchTop(OperationKind operationKind)
{
    if (!m_operationHistory.empty())
    {
        m_operationHistory.pop();
        m_operationHistory.push(operationKind);
    }
}
