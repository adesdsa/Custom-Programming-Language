#include "includes/frontend/semantic_analysis/semantic_checker_facade.h"

bool SemanticCheckerFacade::m_bLoopFlag = false;
size_t SemanticCheckerFacade::m_uLoopBlockCnt = 0;
std::stack<SemanticCheckerFacade::OperationKind> SemanticCheckerFacade::m_operationHistory;
std::stack<SemanticCheckerFacade::BlockKind> SemanticCheckerFacade::m_entryHistory;
bool SemanticCheckerFacade::m_bIsReturnVoid = false;
bool SemanticCheckerFacade::m_bFinishedBlock = false;
bool SemanticCheckerFacade::m_bSuppressDefaultExprCheck = false;

void SemanticCheckerFacade::SetSuppressDefaultExprCheck(bool suppress)
{
    m_bSuppressDefaultExprCheck = suppress;
}

bool SemanticCheckerFacade::CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& varName)
{
    DefaultExpressionChecks();
    return currVarTableNode.FindVar(varName) != std::nullopt;
}

bool SemanticCheckerFacade::CheckFuncDeclaration(const FuncTable& funcTable, const std::string& funcName)
{
    DefaultExpressionChecks();
    FuncMetaData* result = funcTable.FindFunctionByName(funcName);
    if (result == nullptr)
    {
        return false;
    }
    return result->IsFuncDeclared();
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
    size_t uParamIndex = 0;
    std::vector<ExpressionType> parsedExpression;

    // Incase the expression has a single void param.
    if (expression.GetTokens().size() == 1 && expression[FIRST_TOKEN_INDEX].GetValue() == "void")
    {
        return ExpressionType::VOID_TYPE;
    }

    // Incase the expression is a single func call or identifier value.
    if (expression.GetType() == CommandTypes::IDENTIFIER_VALUE)
    {
        auto varData = currVarTableNode.FindVar(expression[IDENTIFIER_INDEX].GetValue());
        if (!varData.has_value())  return ExpressionType::UNKNOWN;

        return TypeToExpressionType(varData.value().type);
    }
    if (expression.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        FuncMetaData* funcData = funcTable.FindFunctionByName(expression[IDENTIFIER_INDEX].GetValue());

        if (funcData) return TypeToExpressionType(funcData->GetRType()); // Saving the func's return type.
        else return ExpressionType::UNKNOWN; //Incase the func doesnt exist
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
                auto varData = currVarTableNode.FindVar(currCommand[IDENTIFIER_INDEX].GetValue());
                if (!varData.has_value())  return ExpressionType::UNKNOWN;

                paramType = TypeToExpressionType(varData.value().type);
            }
            else if (currCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
            {
                FuncMetaData* funcData = funcTable.FindFunctionByName(currCommand[IDENTIFIER_INDEX].GetValue());

                if (funcData) paramType = TypeToExpressionType(funcData->GetRType()); // Saving the func's return type.
                else return ExpressionType::UNKNOWN; //Incase the func doesnt exist
            }
            else if (currCommand.GetType() == CommandTypes::SIZEOF)
            {
                paramType = ExpressionType::MATHEMATICAL; // Sizeof return int.
            }
            else
            {
                return ExpressionType::UNKNOWN; // Undefined behaviour.
            }
            
            parsedExpression.push_back(paramType);
            uParamIndex++;
        }
        else if (expression[i].GetValue() == "void") // Void in the middle of the expression is illegal.
        {
            return ExpressionType::UNKNOWN;
        }
        else if (expression[i].GetValue() == "(")
        {
            parsedExpression.push_back(ExpressionType::PARANTHESES_OPEN);
        }
        else if (expression[i].GetValue() == ")")
        {
            parsedExpression.push_back(ExpressionType::PARANTHESES_CLOSE);
        }
        else // Any const value or operator
        {
            parsedExpression.push_back(TokenToExpressionType(expression[i]));
        }
    }

    parsedExpression = SimplifyExpressionParantheses(parsedExpression);
    return SimplifyExpressionSet(parsedExpression);
}

bool SemanticCheckerFacade::CheckFuncArguments(const std::vector<ExpressionType>& paramTypes, const std::string& sFuncName, const FuncTable& funcTable)
{
    DefaultExpressionChecks();

    const FuncMetaData* func = funcTable.FindFunctionByName(sFuncName);
    if (!func) return false;
    const std::vector<VarMetaData>& funcParams = func->GetFuncParams();

    // Checking that the declaration and given parameters are the same amount.
    if (funcParams.size() != paramTypes.size()) return false;

    if (funcParams.size() == 0) return true; // No args no checks.

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
    return !m_operationHistory.empty();
}

bool SemanticCheckerFacade::CheckFunctionReturnType(const ExpressionType& returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable)
{
    if (!CheckReturnInsideFunc()) return false;
    if (!m_bFinishedBlock)
    {
        SwitchTopOfOperationHistory(OperationKind::Return);
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
    // Use m_bLoopFlag to check if we're inside any loop, not just if the current block is a loop.
    // This correctly handles nested blocks (e.g., if inside for).
    return m_bLoopFlag;
}

bool SemanticCheckerFacade::EnteredBlock(const BlockKind& entryKind, bool bIsReturnVoid)
{
    if (m_bLoopFlag)
    {
        m_uLoopBlockCnt++;
    }

    m_entryHistory.push(entryKind);

    switch (entryKind)
    {
    case BlockKind::Function:
        // Resetting for a new func.
        m_operationHistory = std::stack<OperationKind>();
        m_entryHistory = std::stack<BlockKind>();
        m_entryHistory.push(entryKind); // Reenter value.
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
        if (!m_bLoopFlag)
        {
            m_bLoopFlag = true;
            ++m_uLoopBlockCnt;
        }
        break;

    default:
        break;
    }

    return true;
}

bool SemanticCheckerFacade::ExitedBlock()
{
    if (m_bLoopFlag)
    {
        m_uLoopBlockCnt--;
    }
    
    if (m_operationHistory.empty() || m_entryHistory.empty())
    {
        return true;
    }
    
    OperationKind lastOperation = m_operationHistory.top();

    switch (m_entryHistory.top())
    {
    case BlockKind::Function:
        if (!m_bIsReturnVoid)
        {
            switch (lastOperation)
            {
            // The function stopped with a return.
            case OperationKind::ElseReturn:
                break;
            case OperationKind::Return:
                break;
            // The function stopped without a return.
            default:
                m_operationHistory.pop();
                m_bFinishedBlock = false;
                m_bIsReturnVoid = true;
                return false; // non void func didnt return.
                break;
            }
        }
        m_bFinishedBlock = false;
        m_bIsReturnVoid = true;
        m_operationHistory.pop();
        break;
    
    case BlockKind::If:
        m_operationHistory.pop();
        // If The block ended with a return.
        if (lastOperation == OperationKind::Return ||
            lastOperation == OperationKind::ElseReturn)
        {
            SwitchTopOfOperationHistory(OperationKind::IfReturn);
            m_bFinishedBlock = false;
        }
        else
        {
            SwitchTopOfOperationHistory(OperationKind::If);
            m_bFinishedBlock = false;
        }
        break;

    case BlockKind::Else:
        ElseElifBlockExit(OperationKind::Else);
        break;

    case BlockKind::Elif:
        ElseElifBlockExit(OperationKind::Elif);
        break;

    case BlockKind::Loop:
        if (m_uLoopBlockCnt == 0) m_bLoopFlag = false;
        m_operationHistory.pop();
        // If The block ended with a return.
        if (lastOperation == OperationKind::Return ||
            lastOperation == OperationKind::ElseReturn)
        {
            SwitchTopOfOperationHistory(OperationKind::Return);
        }
        else
        {
            SwitchTopOfOperationHistory(OperationKind::Expression);
        }
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

std::vector<FuncMetaData*> SemanticCheckerFacade::AreAllFunctionsDefined(
    const FuncTable& funcTable
)
{
    std::vector<FuncMetaData*> undefinedFunctions;

    for (const auto& [funcName, funcMetadata] : funcTable.GetAllFunctions())
    {
        if (!funcMetadata->IsFuncDefined())
        {
            undefinedFunctions.push_back(funcMetadata.get());
        }
    }

    return undefinedFunctions;
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
        if (token.GetValue() == "and" ||
            token.GetValue() == "or" ||
            token.GetValue() == "not" ||
            token.GetValue() == "xor")
        {
            return ExpressionType::BITWISE_OPERATOR;
        }
        else if (token.GetValue() == "==" ||
            token.GetValue() == "!=" ||
            token.GetValue() == "<" ||
            token.GetValue() == ">" ||
            token.GetValue() == "<=" ||
            token.GetValue() == ">=")
        {
            return ExpressionType::COMPARRISON_OPERATOR;
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
        expectedType == Type::UNSIGNED_INT_TYPE)
    ) return true;

    return false;
}

inline void SemanticCheckerFacade::DefaultExpressionChecks()
{
    if (m_bSuppressDefaultExprCheck) return;
    if (!m_bFinishedBlock && !m_entryHistory.empty())
    {
        SwitchTopOfOperationHistory(OperationKind::Expression);
    }
}

bool SemanticCheckerFacade::ElseElifBlockEnter()
{
    if (m_operationHistory.empty())
    {
        return true;
    }
    
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
    if (m_operationHistory.empty())
    {
        return;
    }
    
    OperationKind lastOperation = m_operationHistory.top();
    m_operationHistory.pop();
    
    if (m_operationHistory.empty())
    {
        return;
    }
    
    // If The block ended with a return and before that the last condition also returned.
    if (
        (lastOperation == OperationKind::Return ||
        lastOperation == OperationKind::ElseReturn)
        &&
        (m_operationHistory.top() == OperationKind::IfReturn ||
        m_operationHistory.top() == OperationKind::ElifReturn)
    )
    {
        switch (kind)
        {
        case OperationKind::Elif:
            SwitchTopOfOperationHistory(OperationKind::ElifReturn);
            m_bFinishedBlock = false;
            break;
        
        case OperationKind::Else:
            SwitchTopOfOperationHistory(OperationKind::ElseReturn);
            m_bFinishedBlock = true;
            break;
        default:
            break;
        }
    }
    else
    {
        m_bFinishedBlock = false;
        SwitchTopOfOperationHistory(kind);
    }
}

inline void SemanticCheckerFacade::SwitchTopOfOperationHistory(OperationKind operationKind)
{
    if (!m_operationHistory.empty())
    {
        m_operationHistory.pop();
        m_operationHistory.push(operationKind);
    }
    else
    {
        m_operationHistory.push(operationKind);
    }
}

std::vector<ExpressionType> SemanticCheckerFacade::SimplifyExpressionParantheses(std::vector<ExpressionType> parsedExpression)
{
    int openIndex = -1;
    int closeIndex = -1;

    // Find innermost (...)
    for (int i = 0; i < static_cast<int>(parsedExpression.size()); ++i)
    {
        if (parsedExpression[i] == ExpressionType::PARANTHESES_OPEN)
        {
            openIndex = i;
        }
        else if (parsedExpression[i] == ExpressionType::PARANTHESES_CLOSE && openIndex != -1)
        {
            closeIndex = i;
            break;
        }
    }

    if (openIndex == -1 || closeIndex == -1)
    {
        return parsedExpression; // No parentheses left OR unbalanced.
    }

    // Extract inside of parentheses.
    std::vector<ExpressionType> inner(
        parsedExpression.begin() + openIndex + 1,
        parsedExpression.begin() + closeIndex
    );

    ExpressionType result = SimplifyExpressionSet(inner);
    if (result == ExpressionType::UNKNOWN)
    {
        std::vector<ExpressionType> tempVector;
        tempVector.push_back(ExpressionType::UNKNOWN);
        return tempVector;
    }

    // Remove parentheses and content
    parsedExpression.erase(parsedExpression.begin() + openIndex, parsedExpression.begin() + closeIndex + 1);

    // Insert reduced result
    parsedExpression.insert(parsedExpression.begin() + openIndex, result);

    return SimplifyExpressionParantheses(parsedExpression);
}

ExpressionType SemanticCheckerFacade::SimplifyExpressionSet(std::vector<ExpressionType> expressionSet)
{
    ExpressionType currExpressionType = ExpressionType::UNKNOWN;

    for (int i = 0; i < expressionSet.size(); i++)
    {
        // Incase we see math but actually the next operator is a comparrison 
        // operator so we started a comparrison expression => boolean and not math.
        if (expressionSet[i] == ExpressionType::MATHEMATICAL &&
            i + 1 < expressionSet.size() &&
            expressionSet[i + 1] == ExpressionType::COMPARRISON_OPERATOR)
        {
            currExpressionType = ExpressionType::COMPARRISON_OPERATOR;
        }
        // Incase it's first part, it cant start with comparrison.
        else if (currExpressionType == ExpressionType::UNKNOWN && expressionSet[i] != ExpressionType::COMPARRISON_OPERATOR)
        {
            currExpressionType = expressionSet[i];
        }
        // Incase the parts are equals.
        else if (currExpressionType == expressionSet[i])
        {
            continue;
        }
        // Incase it was a boolean untill now and we start a comparrison. e.g.: 1 < 2
        else if (currExpressionType == ExpressionType::BOOLEAN && 
            expressionSet[i] == ExpressionType::MATHEMATICAL && 
            i + 1 < expressionSet.size() && 
            expressionSet[i + 1] == ExpressionType::COMPARRISON_OPERATOR)
        {
            currExpressionType = ExpressionType::COMPARRISON_OPERATOR;
        }
        // Incase we started a comparrison we check if it's ending is legal.
        else if (currExpressionType == ExpressionType::COMPARRISON_OPERATOR && 
            expressionSet[i] == ExpressionType::MATHEMATICAL)
        {
            currExpressionType = ExpressionType::BOOLEAN;
        }
        // Bitwise/logical operators (and, or, xor, not) adapt to operand type:
        //   int op int   -> MATHEMATICAL (bitwise)
        //   bool op bool -> BOOLEAN      (logical)
        else if (
                expressionSet[i] == ExpressionType::BITWISE_OPERATOR &&
                (
                    currExpressionType == ExpressionType::MATHEMATICAL ||
                    currExpressionType == ExpressionType::BOOLEAN
                )
            )
        {
            // keep currExpressionType unchanged – operator inherits from operands
        }
        else if (
                currExpressionType == ExpressionType::BITWISE_OPERATOR &&
                (
                    expressionSet[i] == ExpressionType::MATHEMATICAL ||
                    expressionSet[i] == ExpressionType::BOOLEAN
                )
            )
        {
            currExpressionType = expressionSet[i]; // adopt the operand's type
        }
        // Unknown behaviour.
        else
        {
            return ExpressionType::UNKNOWN;
        }
    }

    return currExpressionType;
}
