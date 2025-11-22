#include "parser/command.h"


Command::Command(): m_sType(CommandTypes::UNIDENTIFIED), m_uParameterIndex(0)
{
}


bool Command::Append(Token currToken, Token nextToken)
{
    m_bIgnoreCurrToken = false;
    bool bReturnValue = false;

    switch (m_sType)
    {
        //todo: rework * derefrencing again
        case CommandTypes::UNIDENTIFIED: bReturnValue = ParseUnidentified(currToken, nextToken);
        case CommandTypes::ACCESS_RIGHTS: bReturnValue = ParseAccessRights(currToken, nextToken);
        case CommandTypes::SCOPE: bReturnValue = ParseScope(currToken, nextToken);
        case CommandTypes::IDENTIFIER: bReturnValue = ParseIdentifier(currToken, nextToken);
        case CommandTypes::FUNC_START: bReturnValue = ParseFuncStart(currToken, nextToken);
        case CommandTypes::FILLED_FUNC_DECLARATION: bReturnValue = ParseFilledFuncDeclaration(currToken, nextToken);
        case CommandTypes::ARROW_FUNC_DECLARATION: bReturnValue = ParseArrowFuncDeclaration(currToken, nextToken);
        case CommandTypes::VAR_DECLARATION_START: bReturnValue = ParseVarDeclarationStart(currToken, nextToken);
        case CommandTypes::ELIF_CALL: bReturnValue = ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::IF_CALL: bReturnValue = ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::FOR_CALL: bReturnValue = ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::WHILE_CALL: bReturnValue = ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::ELIF_START: bReturnValue = ParseParameterKeywordStart(currToken, nextToken);
        case CommandTypes::IF_START: bReturnValue = ParseParameterKeywordStart(currToken, nextToken);
        case CommandTypes::FOR_START: bReturnValue = ParseParameterKeywordStart(currToken, nextToken);
        case CommandTypes::WHILE_START: bReturnValue = ParseParameterKeywordStart(currToken, nextToken);
        case CommandTypes::EXPRESSION_VALUE: bReturnValue = ParseExpressionValue(currToken, nextToken);
        case CommandTypes::POINTER_DECLARATION: bReturnValue = ParsePointerDeclaration(currToken, nextToken);
        case CommandTypes::NOT_FUNC_IDENTIFIER: bReturnValue = ParseNotFuncIdentifier(currToken, nextToken);
        case CommandTypes::ARRAY_ACCESS_START: bReturnValue = ParseArrayAccessStart(currToken, nextToken);
        case CommandTypes::ARRAY_DECLARATION_START: bReturnValue = ParseArrayDeclarationStart(currToken, nextToken);
    }

    if (m_bIgnoreCurrToken) m_tokens.push_back(currToken);
    if (IsValue(nextToken, ";")) bReturnValue = true;

    return bReturnValue;
    // todo: add logs at the end

    // estra rules:
    // there is no support for int a, b, c=0;
    // & * (in the context of pointers) should appear before the identifier when they are called
    // When declaring a pointer the * should appear after the type 
    // In the command class you need to check the pointers
}

bool Command::ParseUnidentified(Token currToken, Token nextToken)
{
    if (IsType(currToken, TokenTypes::IDENTIFIER) && !IsOperator(nextToken)) return NoFinishAndAssignType(CommandTypes::IDENTIFIER);
    else if (IsType(currToken, TokenTypes::KEYWORD))
    {
        if (IsValue(currToken, "void")) return FinishAndAssignType(m_sType = CommandTypes::VOID_COMMAND);
        else if (IsAccessRights(currToken)) return NoFinishAndAssignType(CommandTypes::ACCESS_RIGHTS);
        else if (IsScope(currToken)) return NoFinishAndAssignType(m_sType = CommandTypes::SCOPE);
        else if (IsValue(currToken, "else")) return FinishAndAssignType(m_sType = CommandTypes::ELSE);
        else if (IsValue(currToken, "if")) return CompareNextExpected(nextToken, "(", CommandTypes::IF_CALL);
        else if (IsValue(currToken, "elif")) return CompareNextExpected(nextToken, "(", CommandTypes::ELIF_CALL);
        else if (IsValue(currToken, "for")) return CompareNextExpected(nextToken, "(", CommandTypes::FOR_CALL);
        else if (IsValue(currToken, "while")) return CompareNextExpected(nextToken, "(", CommandTypes::WHILE_CALL);
        else if (IsValue(currToken, "return")) return FinishAndAssignType(CommandTypes::RETURN);
    }
    else if (IsType(currToken, TokenTypes::PUNCTUATION))
    {
        if (IsValue(currToken, ";")) return FinishAndAssignType(CommandTypes::END_OF_COMMAND);
        else if (IsValue(currToken, "{")) return FinishAndAssignType(CommandTypes::BLOCK_START);
        else if (IsValue(currToken, "}")) return FinishAndAssignType(CommandTypes::BLOCK_END);
    }
    else if (IsValue(currToken, "=")) return FinishAndAssignType(CommandTypes::ASSIGNMENT);
    else if (IsExpressionStart(currToken))
    {
        if (IsValue(currToken, "-") || IsValue(currToken, "not")) return NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
        else return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
    }
    else return FoundUnknown();
}

bool Command::ParseIdentifier(Token currToken, Token nextToken)
{
    if (currToken.GetValue() == "(") 
    {
        m_bIgnoreCurrToken = true;
        m_tokens.push_back(currToken);
        m_tokens.push_back(Token(TokenTypes::HAS_PARAM, currToken.GetLine(), m_entityLogger));
        m_uParanthesesCounter++;
        return NoFinishAndAssignType(CommandTypes::FUNC_START);
    }
    else if (currToken.GetValue() == "[") 
    {
        m_bIgnoreCurrToken = true;
        m_tokens.push_back(currToken);
        m_tokens.push_back(Token(TokenTypes::HAS_PARAM, currToken.GetLine(), m_entityLogger));
        m_uParanthesesCounter++;
        return NoFinishAndAssignType(CommandTypes::ARRAY_ACCESS_START);
    }
    else if (currToken.GetValue() == ":") return NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    else return FinishAndAssignType();
}

bool Command::ParseFuncStart(Token currToken, Token nextToken)
{
    m_bIgnoreCurrToken = true;
    if (currToken.GetValue() == ")")
    {
        m_uParanthesesCounter--;
        if (m_uParanthesesCounter == 0)
        {
            m_bIgnoreCurrToken = false;
            //todo: add the next checks:
            //check that the parameters are legal(divided bu ,)
            //check if the func is a calling or a declaration
            //if it's a calling it should be returned as finished command
        }
    }
    else if (currToken.GetValue() == "(")
    {
        m_uParanthesesCounter++;
        return false;
    }
    else if (HandleFuncParams(currToken, nextToken))
    {
        if ((IsValue(nextToken, ")") && m_uParanthesesCounter == 1) || IsValue(nextToken, ",")) return false;
        else return FoundUnknown();
    }
    else return false;

}

bool Command::ParseFilledFuncDeclaration(Token currToken, Token nextToken)
{
    if (IsValue(currToken, "->")) return NoFinishAndAssignType(CommandTypes::ARROW_FUNC_DECLARATION);
    else return FoundUnknown();
}

bool Command::ParseArrowFuncDeclaration(Token currToken, Token nextToken)
{
    if (IsType()) 
    {
        if (IsValue(nextToken, "[")) return NoFinishAndAssignType(CommandTypes::ARRAY_DECLARATION_START);
        return FinishAndAssignType(CommandTypes::FUNC_DECLARATION);
    }
    else return FoundUnknown();
}

bool Command::ParseAccessRights(Token currToken, Token nextToken)
{
    if (IsScope()) return NoFinishAndAssignType(CommandTypes::SCOPE);
    else if (IsType(currToken, TokenTypes::IDENTIFIER)) return NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    else return FoundUnknown();
}

bool Command::ParseScope(Token currToken, Token nextToken)
{
    if (IsType(currToken, TokenTypes::IDENTIFIER)) return NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    else return FoundUnknown();
    return false;
}

bool Command::ParseVarDeclarationStart(Token currToken, Token nextToken)
{
    if (IsType())
    {
        if (IsValue(nextToken, "*")) return NoFinishAndAssignType(CommandTypes::POINTER_DECLARATION);
        else return FinishAndAssignType(CommandTypes::VAR_DECLARATION);
    }
    else return FoundUnknown();
}

bool Command::ParseParameterKeywordCall(Token currToken, Token nextToken)
{
    // todo: rework here
    if (IsValue(currToken, "("))
    {
        m_uParameterIndex++;
        switch(m_sType)
        {
            case CommandTypes::ELIF_CALL: return NoFinishAndAssignType(ELIF_START);
            case CommandTypes::IF_CALL: return NoFinishAndAssignType(IF_START);
            case CommandTypes::FOR_CALL: return NoFinishAndAssignType(FOR_START);
            case CommandTypes::WHILE_CALL: return NoFinishAndAssignType(WHILE_START);
        }
    }
    else return FoundUnknown();
}

bool Command::ParseParameterKeywordStart(Token currToken, Token nextToken)
{
    if (IsValue(currToken, ")")) m_uParameterIndex--;
    else HandleFuncParams(currToken, nextToken); //todo: you need to check this again

    if (m_uParameterIndex == 0)
    {
        switch(m_sType)
        {
            case CommandTypes::ELIF_CALL: return FinishAndAssignType(ELIF);
            case CommandTypes::IF_CALL: return FinishAndAssignType(IF);
            case CommandTypes::FOR_CALL: return FinishAndAssignType(FOR);
            case CommandTypes::WHILE_CALL: return FinishAndAssignType(WHILE);
        }
    }
    else return FoundUnknown();
    return false;
}

bool Command::ParseExpressionValue(Token currToken, Token nextToken)
{
    if (IsTokenExpressionOperator(currToken)) return NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
    else return FoundUnknown();
}

bool Command::ParseExpressionOperator(Token currToken, Token nextToken)
{
    if (m_bBuildingParam)
    {
        m_bIgnoreCurrToken = true;
        if (AddToCurrParam(currToken, nextToken)) 
        {
            m_bBuildingParam = false;
            return FinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else return false;
    }
    else if (IsValueToken(currToken))
    {
        switch (currToken.GetType())
        {
        case TokenTypes::IDENTIFIER:
            if (IsValue(nextToken, "(") || IsValue(nextToken, "["))
            {
                m_tokens.push_back(Token(TokenTypes::HAS_PARAM, currToken.GetLine(), m_entityLogger));
                m_bIgnoreCurrToken = true;
                m_bBuildingParam = true;
                return AddToCurrParam(currToken, nextToken);
            }
            else return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        case TokenTypes::VALUE: return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
    }
    else if (IsValue(currToken, "("))
    {
        m_uParanthesesCounter++; //todo: remember that when it's an expression you need to check that the paranthesesCounter is 0 at the end
        return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
    }
    else if (IsValue(currToken, ")"))
    {
        m_uParanthesesCounter--; //todo: remember that when it's an expression you need to check that the paranthesesCounter is 0 at the end
        return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
    }
    else if (IsValue(currToken, "not") || IsValue(currToken, "*") ||  IsValue(currToken, "&")) return false;
    else return FoundUnknown();

    return false;
}

inline bool Command::ParsePointerDeclaration(Token currToken, Token nextToken)
{
    return FinishAndAssignType(CommandTypes::POINTER_DECLARATION);
}

bool Command::ParseNotFuncIdentifier(Token currToken, Token nextToken)
{
    if (currToken.GetValue() == ":") return NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    else return FinishAndAssignType();
}

bool Command::ParseArrayAccessStart(Token currToken, Token nextToken)
{
    m_bIgnoreCurrToken = true;
    if (currToken.GetValue() == "]")
    {
        m_uParanthesesCounter--;
        if (m_uParanthesesCounter == 0)
        {
            m_bIgnoreCurrToken = false;
            //todo: add the next checks:
            //check that the parameters are legal(divided bu ,)
            //check if the func is a calling or a declaration
            //if it's a calling it should be returned as finished command
        }
    }
    else if (currToken.GetValue() == "[")
    {
        m_uParanthesesCounter++;
        return false;
    }
    else if (AddToCurrParam(currToken, nextToken))
    {
        if ((IsValue(nextToken, "]") && m_uParanthesesCounter == 1)) return false;
        else return FoundUnknown();
    }
    else return false;
}

bool Command::ParseArrayDeclarationStart(Token currToken, Token nextToken)
{
    m_bIgnoreCurrToken = true;
    if (m_uParanthesesCounter == 0)
    {
        m_tokens.push_back(currToken);
        m_tokens.push_back(Token(TokenTypes::HAS_PARAM, currToken.GetLine(), m_entityLogger));
        return false;
    }

    if (currToken.GetValue() == "]")
    {
        m_uParanthesesCounter--;
        if (m_uParanthesesCounter == 0)
        {
            m_bIgnoreCurrToken = false;
            //todo: add the next checks:
            //check that the parameters are legal(divided bu ,)
            //check if the func is a calling or a declaration
            //if it's a calling it should be returned as finished command
        }
    }
    else if (currToken.GetValue() == "[")
    {
        m_uParanthesesCounter++;
        return false;
    }
    else if (AddToCurrParam(currToken, nextToken))
    {
        if ((IsValue(nextToken, "]") && m_uParanthesesCounter == 1)) return false;
        else return FoundUnknown();
    }
    else return false;
}


inline bool Command::IsValue(Token token, std::string sCompare)
{
    return token.GetValue() == sCompare;
}

inline bool Command::IsType(Token token, std::string_view sCompare)
{
    return token.GetType() == sCompare;
}

bool Command::CompareNextExpected(Token token, std::string sCompare, std::string_view sValidType)
{
    if (IsValue(token, sCompare))
    {
        m_sType = sValidType;
        return false;
    }
    else return FoundUnknown();
}

bool Command::IsAccessRights(Token currToken)
{
    return IsValue(currToken, "const") || IsValue(currToken, "readonly");
}

bool Command::IsScope(Token currToken)
{
    return IsValue(currToken, "global") || IsValue(currToken, "var") || IsValue(currToken, "let");
}

bool Command::IsExpressionStart(Token check)
{
    return check.GetType() == TokenTypes::IDENTIFIER || check.GetType() == TokenTypes::VALUE
        || check.GetValue() == "-" ||  || IsValue(currToken, "not");
}

inline bool Command::IsTokenExpressionOperator(Token currToken)
{
    return (IsType(currToken, TokenTypes::OPERATOR) && !(IsValue(currToken, "=") || IsValue(currToken, "&")))
        || IsValue(currToken, "and") || IsValue(currToken, "or") ||
         IsValue(currToken, "xor") || IsValue(currToken, "is");
}

inline bool Command::IsValueToken(Token token)
{
    return IsType(token, TokenTypes::IDENTIFIER) || IsType(token, TokenTypes::VALUE);
}

bool Command::IsOperator(Token token)
{
    return IsValue(token, "and") || IsValue(token, "or") || IsValue(token, "not") || 
    IsValue(token, "xor") || IsValue(token, "is") ||
    (IsType(token, TokenTypes::OPERATOR) && !IsValue(token, "&"));
}


inline bool Command::FinishAndAssignType(std::string_view sType)
{
    m_sType = sType;
    return true;
}

inline bool Command::NoFinishAndAssignType(std::string_view sType)
{
    m_sType = sType;
    return false;
}

inline bool Command::FoundUnknown() 
{
    m_sType = CommandTypes::UNKNOWN;
    return true;
}

bool Command::HandleFuncParams(Token currToken, Token nextToken)
{
    if (IsValue(currToken, ",")) 
    {
        m_tokens.push_back(currToken);
        m_tokens.push_back(Token(TokenTypes::HAS_PARAM, currToken.GetLine(), m_entityLogger));
        m_uParameterIndex++;
        return false;
    }
    else return AddToCurrParam(currToken, nextToken);
}

inline bool Command::AddToCurrParam(Token currToken, Token nextToken)
{
    return m_parameters[m_uParameterIndex].Append(currToken, nextToken);
}

inline void Command::StartParamSequence(Token currToken, Token nextToken)
{
    m_uParanthesesCounter++;
    m_bBuildingParam = true;
    m_bIgnoreCurrToken = true;
    m_tokens.push_back(Token(TokenTypes::HAS_PARAM, currToken.GetLine(), m_entityLogger));
    AddToCurrParam(currToken, nextToken);
}
