#include "parser/command.h"


Command::Command(): m_sType(CommandTypes::UNIDENTIFIED), m_uParameterIndex(0)
{
}


bool Command::Append(Token currToken, Token nextToken)
{
    if (m_sType == CommandTypes::UNIDENTIFIED)
    {
        if (currToken.GetType() == TokenTypes::IDENTIFIER) m_sType = CommandTypes::IDENTIFIER;
        else if (IsAccessRights()) m_sType = CommandTypes::ACCESS_RIGHTS;
        else if (IsScope()) m_sType = CommandTypes::SCOPE;
        else if (currToken.GetValue() == ";") m_sType = CommandTypes::END_OF_COMMAND;
        else if (currToken.GetValue() == "{") m_sType = CommandTypes::BLOCK_START;
        else if (currToken.GetValue() == "{") m_sType = CommandTypes::BLOCK_END;
        else if (currToken.GetValue() == "else") m_sType = CommandTypes::ELSE;
        else if (currToken.GetValue() == "if") m_sType = CommandTypes::IF_CALL;
        else if (currToken.GetValue() == "elif") m_sType = CommandTypes::ELIF_CALL;
        else if (currToken.GetValue() == "for") m_sType = CommandTypes::FOR_CALL;
        else if (currToken.GetValue() == "while") m_sType = CommandTypes::WHILE_CALL;
        else if (currToken.GetValue() == "return") // for now an empty return is illegal grammer
        {
            if (nextToken.GetType() != TokenTypes::VALUE || nextToken.GetType() != TokenTypes::IDENTIFIER || nextToken.GetValue() == "-") return FoundUnknown(); // possible to change this to return void
            else m_sType = CommandTypes::RETURN_START;
        }
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::IDENTIFIER)
    {
        if (currToken.GetValue() == "(") m_sType = CommandTypes::FUNC_START;
        else if (currToken.GetValue() == ":") m_sType = CommandTypes::VAR_DECLARATION_START;
        else if (currToken.GetValue() == "=") m_sType = CommandTypes::VAR_ASSIGNMENT;
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::FUNC_START)
    {
        if (currToken.GetValue() == ")")
        {
            if (CheckParamsValues()) 
            {
                m_sType = CommandTypes::FUNC_CALL;
                return true;
            }
            else if (CheckParamsDeclarations) m_sType = CommandTypes::FILLED_FUNC_DECLARATION;
            else return FoundUnknown();
        }//todo: add the func params handling also handle void
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
        else if (currToken.GetType() == TokenTypes::IDENTIFIER) m_sType = CommandTypes::IDENTIFIER;
        else return FoundUnknown();
    }
    else if (m_sType == CommandTypes::SCOPE)
    {
        if (currToken.GetType() == TokenTypes::IDENTIFIER) m_sType = CommandTypes::IDENTIFIER;
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
    else if (nextToken.GetValue() == "xor"){}//todo: handle
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

    //todo: handle math and boolean
    //todo: handle arrays
    m_tokens.push_back(currToken);
}