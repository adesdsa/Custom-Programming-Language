#include "parser/command.h"


Command::Command(): m_sType(CommandTypes::UNIDENTIFIED), m_uParameterIndex(0)
{
}


bool Command::Append(Token currToken)
{
    if (m_sType == CommandTypes::UNIDENTIFIED)
    {
        if (currToken.GetType() == TokenTypes::IDENTIFIER) m_sType = CommandTypes::IDENTIFIER;
        else
        {
            m_sType = CommandTypes::UNKNOWN;
            return true;
        }
        return false;
    }
    else if (m_sType == CommandTypes::IDENTIFIER)
    {
        if (currToken.GetValue() == "(") m_sType = CommandTypes::FUNC_START;
        else
        {
            m_sType = CommandTypes::UNKNOWN;
            return true;
        }
        return false;
    }
    else if (m_sType == CommandTypes::FUNC_START)
    {
        if (currToken.GetValue() == ")")
        {
            if (CheckParamsCalls()) m_sType = CommandTypes::FUNC_CALL;
            else if (CheckParamsDeclarations) m_sType = CommandTypes::FILLED_FUNC_DECLARATION;
            else
            {
                m_sType = CommandTypes::UNKNOWN;
                return true;
            }
        }//todo: add the func params handling
    }
    else if (m_sType == CommandTypes::FILLED_FUNC_DECLARATION)
    {
        if (currToken.GetValue() == "->") m_sType = CommandTypes::ARROW_FUNC_DECLARATION;
        else
        {
            m_sType = CommandTypes::UNKNOWN;
            return true;
        }
    }
    else if (m_sType == CommandTypes::ARROW_FUNC_DECLARATION)
    {
        if (IsType()) m_sType = CommandTypes::FUNC_DECLARATION;
        else
        {
            m_sType = CommandTypes::UNKNOWN;
            return true;
        }
    }
}