#include "parser/command.h"


Command::Command(): m_sType(CommandTypes::UNIDENTIFIED), m_uParameterIndex(0)
{
}


bool Command::Append(Token currToken, Token nextToken)
{
    m_bIgnoreCurrToken = false;

    switch (m_sType)
    {
        //todo: handle XOR it should be at the top
        //todo: handle CommandTypes::ASSIGNMENT, 
        //todo: remove assignment, it should only be identifier = and then and expression
        //todo: handle void in func params
        //todo: check boolean again
        case CommandTypes::UNIDENTIFIED: return ParseUnidentified(currToken, nextToken);
        case CommandTypes::ACCESS_RIGHTS: return ParseAccessRights(currToken, nextToken);
        case CommandTypes::SCOPE: return ParseScope(currToken, nextToken);
        case CommandTypes::IDENTIFIER: return ParseIdentifier(currToken, nextToken);
        case CommandTypes::FUNC_START: return ParseFuncStart(currToken, nextToken);
        case CommandTypes::FILLED_FUNC_DECLARATION: return ParseFilledFuncDeclaration(currToken, nextToken);
        case CommandTypes::ARROW_FUNC_DECLARATION: return ParseArrowFuncDeclaration(currToken, nextToken);
        case CommandTypes::VAR_DECLARATION_START: return ParseVarDeclarationStart(currToken, nextToken);
        case CommandTypes::ELIF_CALL: return ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::IF_CALL: return ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::FOR_CALL: return ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::WHILE_CALL: return ParseParameterKeywordCall(currToken, nextToken);
        case CommandTypes::ELIF_START: return ParseParameterKeywordStart(currToken, nextToken);
        case CommandTypes::IF_START: return ParseParameterKeywordStart(currToken, nextToken);
        case CommandTypes::FOR_START: return ParseParameterKeywordStart(currToken, nextToken);
        case CommandTypes::WHILE_START: return ParseParameterKeywordStart(currToken, nextToken);
    }

    else if (m_sType == CommandTypes::EXPRESSION_VALUE)
    {
        if (currToken.GetType() == TokenTypes::OPERATOR)
        {
            if (nextToken.GetValue() == "(") m_sType = CommandTypes::PARANTHESES;
            else m_sType = CommandTypes::EXPRESSION_OPERATOR;
        }
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::EXPRESSION_OPERATOR)
    {
        if (m_bBuildingParam) 
        {
            if (!HandleParams(currToken, nextToken))
            {
                if (nextToken.GetValue() != ")") m_sType = CommandTypes::UNKNOWN;
                else m_sType = CommandTypes::EXPRESSION_VALUE;
            }
        }
        else if (currToken.GetType() == TokenTypes::VALUE) m_sType = CommandTypes::EXPRESSION_VALUE;
        else if (currToken.GetType() == TokenTypes::IDENTIFIER)
        {
            if (nextToken.GetValue() == "(")
            {
                m_bBuildingParam = true;
                HandleParams(currToken, nextToken);
            }
            else m_sType = CommandTypes::EXPRESSION_VALUE;
        }
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::PARANTHESES)
    {
        if (currToken.GetValue() == "(")
        {
            if(m_uParanthesesCounter == 0)
            {
                m_uParanthesesCounter++;
                if (!IsExpressionStart(nextToken))
                {
                    m_sType = CommandTypes::UNKNOWN;
                }
                else 
                {
                    m_bBuildingParam = true;
                }
            }
            else m_uParanthesesCounter++;
        }
        else if (currToken.GetValue() == ")")
        {
            m_uParanthesesCounter--;
        }

        if (m_uParanthesesCounter == 0)
        {
            m_bBuildingParam = false;
        }
        if (m_bBuildingParam)
        {
            HandleParantheses(currToken, nextToken);
        }
    }

    //todo: handle assignment
    //todo: handle arrays
    if(!m_bIgnoreCurrToken) m_tokens.push_back(currToken); // there is no support for int a, b, c=0;
    // add logs at the end
}

bool Command::ParseUnidentified(Token currToken, Token nextToken)
{
    if (IsType(currToken, TokenTypes::IDENTIFIER)) TurnToIdentifier(nextToken);
    else if (IsType(currToken, TokenTypes::KEYWORD))
    {
        else if (IsAccessRights(currToken)) m_sType = CommandTypes::ACCESS_RIGHTS;
        else if (IsScope(currToken)) m_sType = CommandTypes::SCOPE;
        else if (IsValue(currToken, "else")) m_sType = CommandTypes::ELSE;
        else if (IsValue(currToken, "if")) CompareNextExpected(nextToken, "(", CommandTypes::IF_CALL);
        else if (IsValue(currToken, "elif")) CompareNextExpected(nextToken, "(", CommandTypes::ELIF_CALL);
        else if (IsValue(currToken, "for")) CompareNextExpected(nextToken, "(", CommandTypes::FOR_CALL);
        else if (IsValue(currToken, "while")) CompareNextExpected(nextToken, "(", CommandTypes::WHILE_CALL);
        else if (IsValue(currToken, "return")) FinishAndAssignType(CommandTypes::RETURN);
    }
    else if (IsType(currToken, TokenTypes::PUNCTUATION))
    {
        else if (IsValue(currToken, ";")) FinishAndAssignType(CommandTypes::END_OF_COMMAND);
        else if (IsValue(currToken, "{")) FinishAndAssignType(CommandTypes::BLOCK_START);
        else if (IsValue(currToken, "}")) FinishAndAssignType(CommandTypes::BLOCK_END);
    }
    else if (IsValue(currToken, "=")) FinishAndAssignType(CommandTypes::ASSIGNMENT);
    else if (IsExpressionStart(currToken))
    {
        if (IsValue(currToken, "-")) m_sType = CommandTypes::EXPRESSION_OPERATOR;
        else m_sType = CommandTypes::EXPRESSION_VALUE;
    }
    else return FoundUnknown();

    return false;
}

bool Command::ParseIdentifier(Token currToken, Token nextToken)
{
    if (currToken.GetValue() == "(") 
    {
        m_sType = CommandTypes::FUNC_START;
        m_uParanthesesCounter++;
    }
    else if (currToken.GetValue() == ":") m_sType = CommandTypes::VAR_DECLARATION_START;
    else if (currToken.GetValue() == "[") // todo: handle this
    {
        m_sType = ARRAY_ACCESS_START;
        m_bIgnoreCurrToken = true;
    }
    else return FinishAndAssignType();

    return false;
}

bool Command::ParseFuncStart(Token currToken, Token nextToken)
{
    if (currToken.GetValue() == ")")
    {
        m_uParameterIndex--;
        if (m_uParameterIndex == 0)
        {
            //todo: add the next checks:
            //check that the parameters are legal(divided bu ,)
            //check if the func is a calling or a declaration
            //if it's a calling it should be returned as finished command
        }
    }
    else HandleParams(currToken, nextToken);
}

bool Command::ParseFilledFuncDeclaration(Token currToken, Token nextToken)
{
    if (IsValue(currToken, "->")) m_sType = CommandTypes::ARROW_FUNC_DECLARATION;
    else return FoundUnknown();
    return false;
}

bool Command::ParseArrowFuncDeclaration(Token currToken, Tpken nextToken)
{
    if (IsType()) return FinishAndAssignType(CommandTypes::FUNC_DECLARATION);
    else return FoundUnknown();
}

bool Command::ParseAccessRights(Token currToken, Token nextToken)
{
    if (IsScope()) m_sType = CommandTypes::SCOPE;
    else if (IsType(currToken, TokenTypes::IDENTIFIER)) TurnToIdentifier(nextToken);
    else return FoundUnknown();
    return false;
}

bool Command::ParseScope(Token currToken, Token nextToken)
{
    if (IsType(currToken, TokenTypes::IDENTIFIER)) TurnToIdentifier(nextToken);
    else return FoundUnknown();
    return false;
}

bool Command::ParseVarDeclarationStart(Token currToken, Token nextToken)
{
    if (IsType()) return FinishAndAssignType(CommandTypes::VAR_DECLARATION);
    else return FoundUnknown();
}

bool Command::ParseParameterKeywordCall(Token currToken, Token nextToken)
{
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
    else HandleParams(currToken, nextToken); //todo: you need to check this again

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

bool Command::FoundUnknown() 
{
    m_sType = CommandTypes::UNKNOWN;
    return true;
}

void Command::HandleParams(Token currToken, Token nextToken)
{
    if (IsValue(currToken, ","))
    {
        m_tokens.push_back(currToken);
        m_uParameterIndex++;
    }
    else if(m_parameters[m_uParameterIndex].Append(currToken, nextToken))
    {
        m_bBuildingParam = false;
        m_uParameterIndex++;
    }
}

bool Command::HandleParantheses(Token currToken, Token nextToken)
{
    if (m_bBuildingParam)
    {
        if(m_parameters[m_uParameterIndex].Append(currToken, nextToken))
        {
            m_bBuildingParam = false;
            m_uParameterIndex++;
        }
    }
    else if (IsExpressionStart(currToken)) // Started building the param
    {
        m_bBuildingParam = true;
        m_tokens.push_back(Token(TokenTypes::HAS_PARAM, currToken.GetLine(), m_entityLogger)); //todo: handle so that the token wont be appended in append()
        if(m_parameters[m_uParameterIndex].Append(currToken, nextToken))
        {
            m_bBuildingParam = false;
            m_uParameterIndex++;
        }
    }
    else return false;

    return true;
}

bool Command::IsExpressionStart(Token check)
{
    return check.GetType() == TokenTypes::IDENTIFIER || check.GetType() == TokenTypes::VALUE || check.GetValue() == "-"
}

bool Command::TurnToIdentifier(Token nextToken)
{
    m_sType = CommandTypes::IDENTIFIER;
    return nextToken.GetType() == TokenTypes::OPERATOR;
}
