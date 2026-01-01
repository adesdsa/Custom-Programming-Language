#include "includes/semantic_analysis/semantic_checker_facade.h"

bool SemanticCheckerFacade::m_bLoopFlag = false;
size_t SemanticCheckerFacade::m_uLoopBlockCnt = 0;
std::vector<SemanticCheckerFacade::OperationKind> SemanticCheckerFacade::m_lastOperations;
size_t SemanticCheckerFacade::m_uCurrentLevel = 0;
std::stack<SemanticCheckerFacade::BlockKind> SemanticCheckerFacade::m_entryHistory;
bool SemanticCheckerFacade::m_bIsReturnVoid = false;

bool SemanticCheckerFacade::CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& varName)
{
    DefaultChecks();
    return currVarTableNode.FindVar(varName) != std::nullopt;
}

bool SemanticCheckerFacade::CheckFuncDeclaration(const FuncTable& funcTable, const std::string& funcName)
{
    DefaultChecks();
    if (funcTable.FindFunctionByName(funcName) == nullptr) return false;
    return funcTable.FindFunctionByName(funcName)->IsFuncDeclared();
}

bool SemanticCheckerFacade::CheckFuncDefinition(const FuncTable& funcTable, const std::string& funcName)
{
    DefaultChecks();
    if (funcTable.FindFunctionByName(funcName) == nullptr) return false;
    return funcTable.FindFunctionByName(funcName)->IsFuncDefined();
}

ExpressionType SemanticCheckerFacade::CheckExpression(
    const Command& expression,
    const FuncTable& funcTable,
    const VarTableNode& currVarTableNode)
{
    DefaultChecks();
    /// @note: The semantic checker doesnt distuinguish between INT and DOUBLE only between MATH and BOOL, That's why it only uses DOUBLE_TYPE
    ExpressionType currType = ExpressionType::UNKNOWN;
    size_t uParamIndex = 0;

    if (expression.GetTokens().size() == 1 && expression[0].GetValue() == "void")
    {
        return ExpressionType::VOID_TYPE;
    }

    for (size_t i = 0; i < expression.GetTokens().size(); i++)
    {
        if (expression[i].GetType() == TokenTypes::HAS_PARAM)
        {
            ExpressionType paramType = ExpressionType::UNKNOWN;
            const Command& currCommand = expression.GetParams()[uParamIndex];
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
            else if (currCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
            {
                FuncMetaData* funcData = funcTable.FindFunctionByName(currCommand[0].GetValue());
                if (funcData) paramType = funcData->GetRType();
                else return ExpressionType::UNKNOWN;
            }
            
            if (currType == ExpressionType::UNKNOWN)
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
            if (currType == ExpressionType::UNKNOWN)
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
    DefaultChecks();
    const std::vector<VarMetaData>& funcParams = funcTable.FindFunctionByName(sFuncName)->GetFuncParams();
    if (funcParams.size() != paramTypes.size()) return false;
    if (funcParams[0].type == Type::VOID_TYPE) return true; /// Void as a param is a hardcoded check inside the parser.
    for (size_t i = 0; i < paramTypes.size(); i++)
    {
        if (!CompareTypes(funcParams[i].type, paramTypes[i])) return false;
    }
    return true;
}

bool SemanticCheckerFacade::CheckFunctionReturnType(const ExpressionType& returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable)
{
    if (!m_lastOperations.empty() &&
        m_lastOperations[m_uCurrentLevel] != OperationKind::ElseReturn &&
        m_lastOperations[m_uCurrentLevel] != OperationKind::Return)
    {
        m_lastOperations[m_uCurrentLevel] = OperationKind::Return;
    }
    return CompareTypes(funcTable.FindFunctionByName(sFuncName)->GetRType(), returnExpressionType);
}

bool SemanticCheckerFacade::CheckAssignment(const std::string& sAssignedVarName, const ExpressionType& assigmentExpressionType, const VarTableNode& currVarTableNode)
{
    DefaultChecks();
    VarMetaData varData = *currVarTableNode.FindVar(sAssignedVarName);
    if (varData.accessRights == AccessRights::CONST_ACCESS || varData.accessRights == AccessRights::READ_ONLY_ACCESS) return false;
    return CompareTypes(varData.type, assigmentExpressionType);
}

bool SemanticCheckerFacade::CheckControlStatement()
{
    return m_entryHistory.top() == BlockKind::Loop;
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
            m_lastOperations.clear();
            while (!m_entryHistory.empty())
            {
                m_entryHistory.pop();
            }
            m_uLoopBlockCnt = 0;
            m_lastOperations.push_back(OperationKind::None);
            m_uCurrentLevel = m_lastOperations.size() - 1;
            m_bIsReturnVoid = bIsReturnVoid;
            m_bLoopFlag = false;
            break;
        
        case BlockKind::If:
            m_lastOperations.push_back(OperationKind::None);
            m_uCurrentLevel = m_lastOperations.size() - 1;
            break;
        
        case BlockKind::Else:
            if (m_lastOperations[m_uCurrentLevel] != OperationKind::If &&
                m_lastOperations[m_uCurrentLevel] != OperationKind::IfReturn &&
                m_lastOperations[m_uCurrentLevel] != OperationKind::Elif &&
                m_lastOperations[m_uCurrentLevel] != OperationKind::ElifReturn)
            {
                return false; // else not after if or elif
            }
            m_lastOperations.push_back(OperationKind::None);
            m_uCurrentLevel = m_lastOperations.size() - 1;
            break;
        
        case BlockKind::Elif:
            if (m_lastOperations[m_uCurrentLevel] != OperationKind::If &&
                m_lastOperations[m_uCurrentLevel] != OperationKind::IfReturn &&
                m_lastOperations[m_uCurrentLevel] != OperationKind::Elif &&
                m_lastOperations[m_uCurrentLevel] != OperationKind::ElifReturn)
            {
                return false; // elif not after if or elif
            }
            m_lastOperations.push_back(OperationKind::None);
            m_uCurrentLevel = m_lastOperations.size() - 1;
            break;

        case BlockKind::Loop:
            m_bLoopFlag = true;
            ++m_uLoopBlockCnt;
            break;

        default:
            break;
        }
    }

    m_entryHistory.push(entryKind);
    return true;
}

bool SemanticCheckerFacade::ExitedBlock()
{
    if (m_bLoopFlag)
    {
        m_uLoopBlockCnt--;
    }

    switch (m_entryHistory.top())
    {
    case BlockKind::Function:
        if (!m_bIsReturnVoid)
        {
            switch (m_lastOperations[m_uCurrentLevel])
            {
            case OperationKind::ElseReturn:
                break;
            
            case OperationKind::Return:
                break;

            default:
                m_lastOperations.clear();
                return false; // non void func didnt return.
                break;
            }
        }
        m_lastOperations.clear();
        break;
    
    case BlockKind::If:
        if (m_lastOperations[m_uCurrentLevel] == OperationKind::Return ||
            m_lastOperations[m_uCurrentLevel] == OperationKind::ElseReturn)
        {
            m_lastOperations[m_uCurrentLevel - 1] = OperationKind::IfReturn;
        }
        else
        {
            m_lastOperations[m_uCurrentLevel - 1] = OperationKind::If;
        }
        --m_uCurrentLevel;
        break;

    case BlockKind::Else:
        if ((m_lastOperations[m_uCurrentLevel] == OperationKind::Return ||
            m_lastOperations[m_uCurrentLevel] == OperationKind::ElseReturn) &&
            (m_lastOperations[m_uCurrentLevel - 1] == OperationKind::IfReturn ||
            m_lastOperations[m_uCurrentLevel - 1] == OperationKind::ElifReturn))
        {
            m_lastOperations[m_uCurrentLevel - 1] = OperationKind::ElseReturn;
        }
        else
        {
            m_lastOperations[m_uCurrentLevel - 1] = OperationKind::Else;
        }
        --m_uCurrentLevel;
        break;

    case BlockKind::Elif:
        if ((m_lastOperations[m_uCurrentLevel] == OperationKind::Return ||
            m_lastOperations[m_uCurrentLevel] == OperationKind::ElseReturn) &&
            (m_lastOperations[m_uCurrentLevel - 1] == OperationKind::IfReturn ||
            m_lastOperations[m_uCurrentLevel - 1] == OperationKind::ElifReturn))
        {
            m_lastOperations[m_uCurrentLevel - 1] = OperationKind::ElifReturn;
        }
        else
        {
            m_lastOperations[m_uCurrentLevel - 1] = OperationKind::Elif;
        }
        --m_uCurrentLevel;
        break;

    case BlockKind::Loop:
        if (m_uLoopBlockCnt == 0) m_bLoopFlag = false;
        break;

    default:
        break;
    }

    m_entryHistory.pop();
    return true;
}

bool SemanticCheckerFacade::IsBlockFinished()
{
    return m_lastOperations[m_uCurrentLevel] == OperationKind::Return ||
        m_lastOperations[m_uCurrentLevel] == OperationKind::ElseReturn;
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
    else if (token.GetValue())
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

void SemanticCheckerFacade::DefaultChecks()
{
    if (!m_bLoopFlag &&
        !m_lastOperations.empty() &&
        m_lastOperations[m_uCurrentLevel] != OperationKind::ElseReturn &&
        m_lastOperations[m_uCurrentLevel] != OperationKind::Return)
    {
        m_lastOperations[m_uCurrentLevel] = OperationKind::Expression;
    }
}
