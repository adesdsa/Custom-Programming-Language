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
        case CommandTypes::UNIDENTIFIED: return ParseUnidentified(currToken, nextToken);
        case CommandTypes::IDENTIFIER: return ParseIdentifier(currToken, nextToken);
        case CommandTypes::FUNC_START: return ParseFuncStart(currToken, nextToken);
    }


    else if (m_sType == CommandTypes::FILLED_FUNC_DECLARATION)
    {
        if (currToken.GetValue() == "->") m_sType = CommandTypes::ARROW_FUNC_DECLARATION;
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::ARROW_FUNC_DECLARATION)
    {
        if (IsType()) 
        {
            m_sType = CommandTypes::FUNC_DECLARATION;
            return true;
        }
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::ACCESS_RIGHTS)
    {
        if (IsScope()) m_sType = CommandTypes::SCOPE;
        else if (currToken.GetType() == TokenTypes::IDENTIFIER) TurnToIdentifier(nextToken);
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::SCOPE)
    {
        if (currToken.GetType() == TokenTypes::IDENTIFIER) TurnToIdentifier(nextToken);
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::VAR_DECLARATION_START)
    {
        if (IsType()) m_sType = CommandTypes::VAR_DECLARATION;
        else return FoundUnknown();

        if(nextToken.GetValue() != "=") return true;
    }
    else if (m_sType == CommandTypes::VAR_DECLARATION)
    {
        //todo: handle assignment parameters and array if needed
        if (CheckParamsValues()) m_sType = CommandTypes::VAR_DECLARATION_ASSIGNMENT;
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::VAR_ASSIGNMENT_START)
    {
        //todo: handle assignment parameters and array if needed
        if (CheckParamsValues()) m_sType = CommandTypes::VAR_ASSIGNMENT;
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::ELIF_CALL || m_sType == CommandTypes::IF_CALL || m_sType == CommandTypes::FOR_CALL || m_sType == CommandTypes::WHILE_CALL)
    {
        if (currToken.GetValue() == "(" && m_sType == CommandTypes::ELIF_CALL) m_sType = CommandTypes::ELIF_START;
        else if (currToken.GetValue() == "(" && m_sType == CommandTypes::IF_CALL) m_sType = CommandTypes::IF_START;
        if (currToken.GetValue() == "(" && m_sType == CommandTypes::FOR_CALL) m_sType = CommandTypes::FOR_START;
        else if (currToken.GetValue() == "(" && m_sType == CommandTypes::WHILE_CALL) m_sType = CommandTypes::WHILE_START;
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::ELIF_START || m_sType == CommandTypes::IF_START || m_sType == CommandTypes::FOR_START || m_sType == CommandTypes::WHILE_START)
    {
        if (currToken.GetValue() == ")" && m_sType == CommandTypes::ELIF_START) m_sType = CommandTypes::ELIF;
        else if (currToken.GetValue() == ")" && m_sType == CommandTypes::IF_START) m_sType = CommandTypes::IF;
        if (currToken.GetValue() == ")" && m_sType == CommandTypes::FOR_START) m_sType = CommandTypes::FOR;
        else if (currToken.GetValue() == ")" && m_sType == CommandTypes::WHILE_START) m_sType = CommandTypes::WHILE;
        else if (false) pass //todo: handle params
        else return FoundUnknown();
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
            if (CheckParamsValues())//todo:change func name
            {
                m_sType = CommandTypes::FUNC_CALL;
                return true;
            }
            else if (CheckParamsDeclarations) m_sType = CommandTypes::FILLED_FUNC_DECLARATION;//todo:change func name
            else return FoundUnknown();
        }
    }//todo: add the func params handling also handle void
    else
    {
        //im here
    }
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

bool Command::FoundUnknown() 
{
    m_sType = CommandTypes::UNKNOWN;
    return true;
}

bool Command::HandleParams(Token currToken, Token nextToken)
{
    if (m_bBuildingParam && currToken.GetValue() == ",")
    {
        m_tokens.push_back(currToken);
        m_uParameterIndex++;
    }
    else if (m_bBuildingParam)
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
    else if (currToken.GetValue() != ",") return false;

    return true;
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
