#include "parser/command.h"


Command::Command(): m_sType(CommandTypes::UNIDENTIFIED), m_uParameterIndex(0)
{
}


bool Command::Append(Token currToken)
{
    if (m_sType == CommandTypes::UNIDENTIFIED)
    {
        if (currToken.GetType() == TokenTypes::IDENTIFIER) m_sType = CommandTypes::IDENTIFIER;
        else if (IsAccessRights()) m_sType = CommandTypes::ACCESS_RIGHTS;
        else if (IsScope()) m_sType = CommandTypes::SCOPE;
        else FoundUnknown();
        return false;
    }
    else if (m_sType == CommandTypes::IDENTIFIER)
    {
        if (currToken.GetValue() == "(") m_sType = CommandTypes::FUNC_START;
        else if (currToken.GetValue() == ":") m_sType = CommandTypes::VAR_DECLARATION_START;
        else FoundUnknown();
        return false;
    }
    else if (m_sType == CommandTypes::FUNC_START)
    {
        if (currToken.GetValue() == ")")
        {
            if (CheckParamsCalls()) m_sType = CommandTypes::FUNC_CALL;
            else if (CheckParamsDeclarations) m_sType = CommandTypes::FILLED_FUNC_DECLARATION;
            else FoundUnknown();
        }//todo: add the func params handling
    }
    else if (m_sType == CommandTypes::FILLED_FUNC_DECLARATION)
    {
        if (currToken.GetValue() == "->") m_sType = CommandTypes::ARROW_FUNC_DECLARATION;
        else FoundUnknown();
    }
    else if (m_sType == CommandTypes::ARROW_FUNC_DECLARATION)
    {
        if (IsType()) m_sType = CommandTypes::FUNC_DECLARATION;
        else FoundUnknown();
    }
    else if (m_sType == CommandTypes::ACCESS_RIGHTS)
    {
        if (IsScope()) m_sType = CommandTypes::SCOPE;
        else if (currToken.GetType() == TokenTypes::IDENTIFIER) m_sType = CommandTypes::IDENTIFIER;
        else FoundUnknown();
    }
    else if (m_sType == CommandTypes::SCOPE)
    {
        if (currToken.GetType() == TokenTypes::IDENTIFIER) m_sType = CommandTypes::IDENTIFIER;
        else FoundUnknown();
    }
    else if (m_sType == CommandTypes::VAR_DECLARATION_START)
    {
        if (IsType()) m_sType = VAR_DECLARATION;
        else FoundUnknown();
    }
}