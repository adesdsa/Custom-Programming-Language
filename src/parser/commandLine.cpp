#include "parser/commandLine.h"

CommandLine::CommandLine(const EntityLogger& entityLogger): 
m_entityLogger(entityLogger), m_bFinished(false), m_bLegal(true)
{
    m_line.push_back(Command());
}

bool CommandLine::IsFinished() const
{
    return m_bFinished;
}

bool CommandLine::IsLegal() const
{
    return m_bLegal;
}

const std::vector<Command>& CommandLine::GetLine() const
{
    return m_line;
}

bool CommandLine::AddToken(Token& currToken, Token& nextToken)
{
    if (!m_bFinished)
    {
        if (m_line[m_uLineIndex].Append(currToken, nextToken))
        {
            if (m_line[m_uLineIndex].GetType() == CommandTypes::END_OF_COMMAND
            || m_line[m_uLineIndex].GetType() == CommandTypes::BLOCK_START
            || m_line[m_uLineIndex].GetType() == CommandTypes::BLOCK_END) 
                m_bFinished =true;
            else if (m_line[m_uLineIndex].GetType() == CommandTypes::UNKNOWN)
                m_bLegal = false;
            else {
                ReformCommand(m_line[m_uLineIndex]);
                m_uLineIndex++;
                m_line.push_back(Command());
            }
        }
    }

    if (m_bFinished && m_bLegal)
    {
        CheckLine();
        return true;
    }
    return false;
}

void CommandLine::CheckLine()
{
    bool bLegalLine = false;

    if (m_line[0].GetType() == CommandTypes::IDENTIFIER || m_line[0].GetType() == CommandTypes::ARRAY_ACCESS)
    {
        if (m_line[1].GetType() == CommandTypes::ASSIGNMENT)
            if (m_line[2].IsExpression())
                if (m_line[3].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::ELSE)
    {
        if (m_line[1].GetType() == CommandTypes::BLOCK_START) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::RETURN)
    {
        if (m_line[1].IsExpression() || m_line[1].GetType() == CommandTypes::VOID_COMMAND) 
            if (m_line[2].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::POINTER_DECLARATION || m_line[0].GetType() == CommandTypes::VAR_DECLARATION)
    {
        if (m_line[1].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
        else if (m_line[1].GetType() == CommandTypes::ASSIGNMENT)
            if (m_line[2].IsExpression())
                if (m_line[3].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::CONTROL_STATEMENT)
    {
        if (m_line[1].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::ELIF ||
        m_line[0].GetType() == CommandTypes::IF ||
        m_line[0].GetType() == CommandTypes::FOR ||
        m_line[0].GetType() == CommandTypes::WHILE)
    {
        if (m_line[1].GetType() == CommandTypes::BLOCK_START) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::FINISHED_FUNC_DECLARATION)
    {
        if (m_line[1].GetType() == CommandTypes::BLOCK_START || m_line[1].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        if (m_line[1].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::ARRAY_DECLARATION)
    {
        // todo: make it so that the value can only be an array value e.g. (1 , 2, 3) or something like that
        if (m_line[1].GetType() == CommandTypes::ASSIGNMENT)
            if (m_line[2].IsExpression())
                if (m_line[3].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    
    if (bLegalLine) m_bLegal = true;
}

void CommandLine::ReformCommand(Command& checkedCommand)
{
    // todo: make sure that in funcs and for you can have a few commands in the params
    if (checkedCommand.GetType() == CommandTypes::ELIF)
    {
        if (checkedCommand.GetTokens().size() != 4) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else 
        {
            Command innerCommand = checkedCommand.GetParams()[0];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::IF)
    {
        if (checkedCommand.GetTokens().size() != 4) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else 
        {
            Command innerCommand = checkedCommand.GetParams()[0];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::FOR)
    {
        if (checkedCommand.GetTokens().size() != 10) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else 
        {
            // todo: add a rule that a for loop must be of the next preset:
            // for ( var_declaration/identifier assigmnet value ; expression ; expression )
            // Checking if one of the Token's types are wrong
            if (checkedCommand[2].GetType() != TokenTypes::HAS_PARAM ||
            checkedCommand[3].GetType() != TokenTypes::HAS_PARAM ||
            checkedCommand[4].GetType() != TokenTypes::HAS_PARAM ||
            checkedCommand[5].GetValue() != ";" ||
            checkedCommand[6].GetType() != TokenTypes::HAS_PARAM ||
            checkedCommand[7].GetValue() != ";" ||
            checkedCommand[8].GetType() != TokenTypes::HAS_PARAM)
                checkedCommand.SetType(CommandTypes::UNKNOWN);

            Command innerCommand = checkedCommand.GetParams()[2];
            if (innerCommand.GetType() != CommandTypes::VAR_DECLARATION &&
            innerCommand.GetType() != CommandTypes::IDENTIFIER)
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }

            if (!m_bLegal) return;

            innerCommand = checkedCommand.GetParams()[3];
            if (innerCommand.GetType() != CommandTypes::ASSIGNMENT)
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }

            if (!m_bLegal) return;

            innerCommand = checkedCommand.GetParams()[4];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }

            if (!m_bLegal) return;

            innerCommand = checkedCommand.GetParams()[6];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }

            if (!m_bLegal) return;

            innerCommand = checkedCommand.GetParams()[8];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::WHILE)
    {
        if (checkedCommand.GetTokens().size() != 4) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else 
        {
            Command innerCommand = checkedCommand.GetParams()[0];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression()) m_bLegal = false;
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_VALUE)
    {
        int iParamIndex = 0;

        if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        for (int i = 0; i < checkedCommand.GetTokens().size() && m_bLegal; i++)
        {
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
            {
                Command innerCommand = checkedCommand.GetParams()[iParamIndex];
                ReformCommand(innerCommand);
                if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
                {
                    m_bLegal = false;
                    checkedCommand.SetType(CommandTypes::UNKNOWN);
                }
                iParamIndex++;
            }
        }

        if (m_bLegal) 
        {
            checkedCommand.SetType(CommandTypes::EXPRESSION);
            checkedCommand.SetExpression(true);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_OPERATOR)
    {
        int iParamIndex = 0;

        if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        for (int i = 0; i < checkedCommand.GetTokens().size() && m_bLegal; i++)
        {
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
            {
                Command innerCommand = checkedCommand.GetParams()[iParamIndex];
                ReformCommand(innerCommand);
                if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
                {
                    m_bLegal = false;
                    checkedCommand.SetType(CommandTypes::UNKNOWN);
                }
                iParamIndex++;
            }
        }

        if (!m_bLegal) return;

        Token lastToken = checkedCommand[checkedCommand.GetTokens().size() - 1];
        if (lastToken.GetValue() != ")")
        {
            m_bLegal = false;
            checkedCommand.SetType(CommandTypes::UNKNOWN);
        }

        if (m_bLegal) 
        {
            checkedCommand.SetType(CommandTypes::EXPRESSION);
            checkedCommand.SetExpression(true);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_DECLARATION)
    {
        int iParamIndex = 0;
        bool bExpectingExpression = true;

        if (checkedCommand.GetTokens().size() == 4)
        {
            if (checkedCommand.GetParams()[2].GetType() == CommandTypes::VOID_COMMAND)
                return;
        }

        for (int i = 2; i < checkedCommand.GetTokens().size() - 1 && m_bLegal; i++)
        {
            if (bExpectingExpression)
            {
                Command innerCommand = checkedCommand.GetParams()[iParamIndex];
                if (innerCommand.GetType() != CommandTypes::VAR_DECLARATION)
                {
                    m_bLegal = false;
                    checkedCommand.SetType(CommandTypes::UNKNOWN);
                }
                iParamIndex++;
            }
            else if (checkedCommand[i].GetValue() != ",") 
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }
        }

        if (checkedCommand[checkedCommand.GetTokens().size() - 2].GetValue() == ",")
        {
            m_bLegal = false;
            checkedCommand.SetType(CommandTypes::UNKNOWN);
        }

    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        int iParamIndex = 0;
        bool bExpectingExpression = true;

        if (checkedCommand.GetTokens().size() == 4)
        {
            if (checkedCommand.GetParams()[2].GetType() == CommandTypes::VOID_COMMAND)
            {
                checkedCommand.SetExpression(true);
                return;
            }
        }

        for (int i = 2; i < checkedCommand.GetTokens().size() - 1 && m_bLegal; i++)
        {
            if (bExpectingExpression)
            {
                Command innerCommand = checkedCommand.GetParams()[iParamIndex];
                ReformCommand(innerCommand);
                if (!innerCommand.IsExpression() || innerCommand.GetType() == TokenTypes::UNKNOWN) 
                {
                    m_bLegal = false;
                    checkedCommand.SetType(CommandTypes::UNKNOWN);
                }
                iParamIndex++;
            }
            else if (checkedCommand[i].GetValue() != ",") 
            {
                m_bLegal = false;
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }
        }

        if (checkedCommand[checkedCommand.GetTokens().size() - 2].GetValue() == ",")
        {
            m_bLegal = false;
            checkedCommand.SetType(CommandTypes::UNKNOWN);
        }
        
        if (m_bLegal) checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_ACCESS)
    {
        if (checkedCommand.GetTokens().size() != 4) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else 
        {
            Command innerCommand = checkedCommand.GetParams()[0];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression()) m_bLegal = false;
        }

        if (m_bLegal) checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_DECLARATION)
    {
        if (checkedCommand.GetTokens().size() != 4) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else 
        {
            Command innerCommand = checkedCommand.GetParams()[0];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression()) m_bLegal = false;
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::IDENTIFIER)
    {
        checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::UNIDENTIFIED ||
    checkedCommand.GetType() == CommandTypes::FUNC_START ||
    checkedCommand.GetType() == CommandTypes::FILLED_FUNC_DECLARATION ||
    checkedCommand.GetType() == CommandTypes::ARROW_FUNC_DECLARATION ||
    checkedCommand.GetType() == CommandTypes::ACCESS_RIGHTS ||
    checkedCommand.GetType() == CommandTypes::SCOPE ||
    checkedCommand.GetType() == CommandTypes::VAR_DECLARATION_START ||
    checkedCommand.GetType() == CommandTypes::ELIF_CALL ||
    checkedCommand.GetType() == CommandTypes::IF_CALL ||
    checkedCommand.GetType() == CommandTypes::ELIF_START ||
    checkedCommand.GetType() == CommandTypes::IF_START ||
    checkedCommand.GetType() == CommandTypes::FOR_CALL ||
    checkedCommand.GetType() == CommandTypes::WHILE_CALL ||
    checkedCommand.GetType() == CommandTypes::FOR_START ||
    checkedCommand.GetType() == CommandTypes::WHILE_START ||
    checkedCommand.GetType() == CommandTypes::ARRAY_ACCESS_START ||
    checkedCommand.GetType() == CommandTypes::NOT_FUNC_IDENTIFIER ||
    checkedCommand.GetType() == CommandTypes::ARRAY_DECLARATION_START)
    {
        m_bLegal = false;
        checkedCommand.SetType(CommandTypes::UNKNOWN);
    }
}
