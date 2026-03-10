#include "includes/frontend/semantic_analysis/semantic_checker_facade.h"

SemanticCheckerFacade::SemanticCheckerFacade() :
    m_bLoopFlag(false),
    m_bIsReturnVoid(false),
    m_bFinishedBlock(false),
    m_bSuppressDefaultExprCheck(false),
    m_uLoopBlockCnt(0)
{}

void SemanticCheckerFacade::SetSuppressDefaultExprCheck(bool suppress)
{
    this->m_bSuppressDefaultExprCheck = suppress;
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

bool SemanticCheckerFacade::IsFirstTokenStringOrCharLike(const Command& expression, const FuncTable& funcTable, const VarTableNode& currVarTableNode)
{
    if (expression.GetTokens().empty()) return false;
    size_t paramIndex = 0;
    return IsFirstTokenStringOrCharLikeFrom(expression, 0, expression.GetTokens().size(), paramIndex, funcTable, currVarTableNode);
}

bool SemanticCheckerFacade::IsFirstTokenStringOrCharLikeFrom(const Command& expression, size_t start, size_t end, size_t& paramIndex,
    const FuncTable& funcTable, const VarTableNode& currVarTableNode)
{
    if (start >= end) return false;
    const Token& first = expression[start];
    if (first.GetType() == TokenTypes::STRING_LITERAL) return true;
    if (first.GetType() == TokenTypes::CHAR_LITERAL) return true;
    if (first.GetValue() == "(")
    {
        int depth = 0;
        size_t closeIdx = start;
        for (size_t i = start; i < end; ++i)
        {
            if (expression[i].GetValue() == "(") ++depth;
            else if (expression[i].GetValue() == ")")
            {
                --depth;
                if (depth == 0) { closeIdx = i; break; }
            }
        }
        if (closeIdx > start + 1)
            return IsFirstTokenStringOrCharLikeFrom(expression, start + 1, closeIdx, paramIndex, funcTable, currVarTableNode);
        return false;
    }
    if (first.GetType() == TokenTypes::HAS_PARAM && paramIndex < expression.GetParams().size())
    {
        const Command& param = expression.GetParams()[paramIndex++];
        if (param.GetType() == CommandTypes::IDENTIFIER_VALUE)
        {
            auto varData = currVarTableNode.FindVar(param[IDENTIFIER_INDEX].GetValue());
            return varData.has_value() && (varData->type == Type::STRING_TYPE || varData->type == Type::CHAR_TYPE);
        }
        if (param.GetType() == CommandTypes::FINISHED_FUNC_CALL)
        {
            FuncMetaData* f = funcTable.FindFunctionByName(param[IDENTIFIER_INDEX].GetValue());
            return f && (f->GetRType() == Type::STRING_TYPE || f->GetRType() == Type::CHAR_TYPE);
        }
        if (param.GetType() == CommandTypes::EXPRESSION)
            return IsFirstTokenStringOrCharLike(param, funcTable, currVarTableNode);
    }
    return false;
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

    // Incase the expression is string related (string/char literal, string/char var, string + string, char + string, etc.)
    if (expression.GetTokens().size() >= 1 && IsFirstTokenStringOrCharLike(expression, funcTable, currVarTableNode))
    {
        bool sawString = false;
        for (size_t i = 0; i < expression.GetTokens().size(); )
        {
            if (expression[i].GetType() == TokenTypes::STRING_LITERAL)
            {
                sawString = true;
                if (i + 1 < expression.GetTokens().size() && expression[i + 1].GetValue() != "+")
                    return ExpressionType::UNKNOWN;
                i += 2; // skip operand and the "+" after it (or end)
                continue;
            }
            else if (expression[i].GetType() == TokenTypes::CHAR_LITERAL)
            {
                if (i + 1 < expression.GetTokens().size() && expression[i + 1].GetValue() != "+")
                    return ExpressionType::UNKNOWN;
                i += 2;
                continue;
            }
            else if (expression[i].GetValue() == "(")
            {
                int depth = 0;
                size_t closeIdx = i;
                for (size_t j = i; j < expression.GetTokens().size(); ++j)
                {
                    if (expression[j].GetValue() == "(") ++depth;
                    else if (expression[j].GetValue() == ")") { --depth; if (depth == 0) { closeIdx = j; break; } }
                }
                if (closeIdx <= i + 1) return ExpressionType::UNKNOWN;
                // Build sub-expression from content and recurse
                std::vector<Token> innerTokens(expression.GetTokens().begin() + i + 1, expression.GetTokens().begin() + closeIdx);
                Command inner;
                inner.SetType(CommandTypes::EXPRESSION);
                inner.SetTokens(std::move(innerTokens));
                size_t paramCount = 0;
                for (size_t k = i + 1; k < closeIdx; ++k)
                    if (expression[k].GetType() == TokenTypes::HAS_PARAM) ++paramCount;
                for (size_t p = 0; p < paramCount && uParamIndex < expression.GetParams().size(); ++p)
                    inner.GetParams().push_back(expression.GetParams()[uParamIndex++]);
                ExpressionType innerType = CheckExpression(inner, funcTable, currVarTableNode);
                if (innerType == ExpressionType::STRING) sawString = true;
                if (innerType != ExpressionType::STRING && innerType != ExpressionType::CHAR) return ExpressionType::UNKNOWN;
                i = closeIdx + 1;
                if (i < expression.GetTokens().size() && expression[i].GetValue() == "+") ++i;
                continue;
            }
            else if (expression[i].GetType() == TokenTypes::HAS_PARAM)
            {
                if (uParamIndex >= expression.GetParams().size()) return ExpressionType::UNKNOWN;
                const Command& currCommand = expression.GetParams()[uParamIndex++];
                if (currCommand.GetType() == CommandTypes::IDENTIFIER_VALUE)
                {
                    auto varData = currVarTableNode.FindVar(currCommand[IDENTIFIER_INDEX].GetValue());
                    if (!varData.has_value())  return ExpressionType::UNKNOWN;

                    if (varData.value().type == Type::STRING_TYPE) sawString = true;
                    if (varData.value().type == Type::STRING_TYPE || varData.value().type == Type::CHAR_TYPE)
                    {
                        if (i + 1 < expression.GetTokens().size() && expression[i + 1].GetValue() != "+")
                            return ExpressionType::UNKNOWN;
                        i += 2;
                        continue;
                    }
                }
                else if (currCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
                {
                    FuncMetaData* funcData = funcTable.FindFunctionByName(currCommand[IDENTIFIER_INDEX].GetValue());

                    if (funcData && (funcData->GetRType() == Type::STRING_TYPE || funcData->GetRType() == Type::CHAR_TYPE))
                    {
                        if (funcData->GetRType() == Type::STRING_TYPE) sawString = true;
                        if (i + 1 < expression.GetTokens().size() && expression[i + 1].GetValue() != "+")
                            return ExpressionType::UNKNOWN;
                        i += 2;
                        continue;
                    }
                }
                return ExpressionType::UNKNOWN;
            }
            else if (expression[i].GetValue() == "+")
            {
                ++i;
                continue;
            }
            return ExpressionType::UNKNOWN;
        }
        // Only return STRING if we have actual string concatenation; single char stays MATHEMATICAL (for return type match)
        return sawString ? ExpressionType::STRING : ExpressionType::CHAR;
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

    // built-in println accepts any printable type: int, double, bool, string.
    if (sFuncName == "println" && paramTypes.size() == 1)
    {
        ExpressionType argType = paramTypes[0];
        if (argType == ExpressionType::MATHEMATICAL ||
            argType == ExpressionType::BOOLEAN ||
            argType == ExpressionType::STRING ||
            argType == ExpressionType::CHAR)
        {
            return true;
        }
    }

    // Checking that all of the parameter types are the right function declared types.
    for (size_t i = 0; i < paramTypes.size(); i++)
    {
        if (!CompareTypes(funcParams[i].type, paramTypes[i])) return false;
    }
    return true;
}

bool SemanticCheckerFacade::CheckReturnInsideFunc()
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
    auto found = currVarTableNode.FindVar(sAssignedVarName);
    if (!found.has_value()) return false;
    VarMetaData varData = *found;
    if (varData.accessRights == AccessRights::CONST_ACCESS || varData.accessRights == AccessRights::READ_ONLY_ACCESS) return false;
    return CompareTypes(varData.type, assigmentExpressionType);
}

bool SemanticCheckerFacade::CheckControlStatement()
{
    // Use m_bLoopFlag to check if we're inside any loop, not just if the current block is a loop.
    // This correctly handles nested blocks (e.g., if inside for).
    return this->m_bLoopFlag;
}

bool SemanticCheckerFacade::EnteredBlock(const BlockKind& entryKind, bool bIsReturnVoid)
{
    if (this->m_bLoopFlag)
    {
        this->m_uLoopBlockCnt++;
    }

    this->m_entryHistory.push(entryKind);

    switch (entryKind)
    {
    case BlockKind::Function:
        // Resetting for a new func.
        this->m_operationHistory = std::stack<OperationKind>();
        this->m_entryHistory = std::stack<BlockKind>();
        this->m_entryHistory.push(entryKind); // Reenter value.
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
        break;
    
    case BlockKind::Elif:
        if (ElseElifBlockEnter()) return false;
        break;

    case BlockKind::Loop:
        if (!this->m_bLoopFlag)
        {
            this->m_bLoopFlag = true;
            ++this->m_uLoopBlockCnt;
        }
        break;

    default:
        break;
    }

    return true;
}

bool SemanticCheckerFacade::ExitedBlock()
{
    if (this->m_bLoopFlag)
    {
        this->m_uLoopBlockCnt--;
    }
    
    if (this->m_operationHistory.empty() || this->m_entryHistory.empty())
    {
        return true;
    }
    
    OperationKind lastOperation = this->m_operationHistory.top();

    switch (this->m_entryHistory.top())
    {
    case BlockKind::Function:
        if (!this->m_bIsReturnVoid)
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
                this->m_operationHistory.pop();
                this->m_bFinishedBlock = false;
                this->m_bIsReturnVoid = true;
                return false; // non void func didnt return.
                break;
            }
        }
        this->m_bFinishedBlock = false;
        this->m_bIsReturnVoid = true;
        this->m_operationHistory.pop();
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

    case BlockKind::Loop:
        if (this->m_uLoopBlockCnt == 0) this->m_bLoopFlag = false;
        this->m_operationHistory.pop();
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

    this->m_entryHistory.pop();
    return true;
}

bool SemanticCheckerFacade::IsBlockFinished()
{
    return !this->m_operationHistory.empty() &&
        (this->m_operationHistory.top() == OperationKind::Return ||
        this->m_operationHistory.top() == OperationKind::ElseReturn);
}

std::vector<FuncMetaData*> SemanticCheckerFacade::AreAllFunctionsDefined(
    const FuncTable& funcTable
)
{
    std::vector<FuncMetaData*> undefinedFunctions;

    for (const auto& [funcName, funcMetadata] : funcTable.GetAllFunctions())
    {
        if (!funcMetadata->IsExtern() && !funcMetadata->IsFuncDefined())
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
    else if (type == Type::STRING_TYPE) return ExpressionType::STRING;
    else if (type == Type::CHAR_TYPE) return ExpressionType::CHAR;
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
    else if (token.GetType() == TokenTypes::STRING_LITERAL)
    {
        return ExpressionType::STRING;
    }
    else if (token.GetType() == TokenTypes::CHAR_LITERAL)
    {
        return ExpressionType::CHAR;
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
    if (expectedType == Type::STRING_TYPE && (gottenType == ExpressionType::STRING || gottenType == ExpressionType::CHAR)) return true;  // char can assign to string
    if (expectedType == Type::CHAR_TYPE && gottenType == ExpressionType::CHAR) return true;
    if (gottenType == ExpressionType::MATHEMATICAL &&
        (expectedType == Type::DOUBLE_TYPE ||
        expectedType == Type::INT_TYPE ||
        expectedType == Type::UNSIGNED_INT_TYPE ||
        expectedType == Type::CHAR_TYPE)
    ) return true;

    return false;
}

inline void SemanticCheckerFacade::DefaultExpressionChecks()
{
    if (this->m_bSuppressDefaultExprCheck) return;
    if (!this->m_bFinishedBlock && !this->m_entryHistory.empty())
    {
        SwitchTopOfOperationHistory(OperationKind::Expression);
    }
}

bool SemanticCheckerFacade::ElseElifBlockEnter()
{
    if (this->m_operationHistory.empty())
    {
        return true;
    }
    
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
    if (this->m_operationHistory.empty())
    {
        return;
    }
    
    OperationKind lastOperation = this->m_operationHistory.top();
    this->m_operationHistory.pop();
    
    if (this->m_operationHistory.empty())
    {
        return;
    }
    
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

    for (size_t i = 0; i < expressionSet.size(); i++)
    {
        // Incase we see math/char but actually the next operator is a comparrison 
        // operator so we started a comparrison expression => boolean and not math.
        if ((expressionSet[i] == ExpressionType::MATHEMATICAL || expressionSet[i] == ExpressionType::CHAR) &&
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
            (expressionSet[i] == ExpressionType::MATHEMATICAL || expressionSet[i] == ExpressionType::CHAR) && 
            i + 1 < expressionSet.size() && 
            expressionSet[i + 1] == ExpressionType::COMPARRISON_OPERATOR)
        {
            currExpressionType = ExpressionType::COMPARRISON_OPERATOR;
        }
        // Incase we started a comparrison we check if it's ending is legal.
        else if (currExpressionType == ExpressionType::COMPARRISON_OPERATOR && 
            (expressionSet[i] == ExpressionType::MATHEMATICAL || expressionSet[i] == ExpressionType::CHAR))
        {
            currExpressionType = ExpressionType::BOOLEAN;
        }
        // Arithmetic after a comparison is part of the right-hand side
        // due to operator precedence (e.g. j < height - i - 1).
        else if (currExpressionType == ExpressionType::BOOLEAN &&
            (expressionSet[i] == ExpressionType::MATHEMATICAL || expressionSet[i] == ExpressionType::CHAR))
        {
            // stay BOOLEAN
        }
        // Incase we have boolean comparrison
        else if (currExpressionType == ExpressionType::BOOLEAN &&
            expressionSet[i] == ExpressionType::COMPARRISON_OPERATOR &&
            i + 1 < expressionSet.size() &&
            (expressionSet[i + 1] == ExpressionType::BOOLEAN || expressionSet[i + 1] == ExpressionType::MATHEMATICAL || expressionSet[i + 1] == ExpressionType::CHAR))
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
                    currExpressionType == ExpressionType::BOOLEAN ||
                    currExpressionType == ExpressionType::CHAR
                )
            )
        {
            // keep currExpressionType unchanged – operator inherits from operands
        }
        else if (
                currExpressionType == ExpressionType::BITWISE_OPERATOR &&
                (
                    expressionSet[i] == ExpressionType::MATHEMATICAL ||
                    expressionSet[i] == ExpressionType::BOOLEAN ||
                    expressionSet[i] == ExpressionType::CHAR
                )
            )
        {
            currExpressionType = expressionSet[i]; // adopt the operand's type
        }
        // CHAR in arithmetic: char + int, int + char, char + char -> MATHEMATICAL
        else if ((currExpressionType == ExpressionType::CHAR && expressionSet[i] == ExpressionType::MATHEMATICAL) ||
                 (currExpressionType == ExpressionType::MATHEMATICAL && expressionSet[i] == ExpressionType::CHAR) ||
                 (currExpressionType == ExpressionType::CHAR && expressionSet[i] == ExpressionType::CHAR))
        {
            currExpressionType = ExpressionType::MATHEMATICAL;
        }
        // Unknown behaviour.
        else
        {
            return ExpressionType::UNKNOWN;
        }
    }

    return currExpressionType;
}
