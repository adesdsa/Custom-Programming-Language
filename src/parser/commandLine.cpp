#include "parser/commandLine.h"

CommandLine::CommandLine(const EntityLogger& entityLogger): 
m_entityLogger(entityLogger), m_bFinished(false), m_bLegal(true)
{
    m_line.push_back(Command(m_entityLogger));
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
                m_line.push_back(Command(m_entityLogger));
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
    // todo: handle everthying here: all needed to be done is check that the first command and second command are legal and can be together and so on untill the end of the command.
    /*for (int i = 0; i < m_line.size() && ; i++)
    {
        switch (i)
        {
        case 0:
            
            break;
        
        default:
            break;
        }
    }*/
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
            if (!innerCommand.IsExpression()) m_bLegal = false;
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
            if (!innerCommand.IsExpression()) m_bLegal = false;
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
                m_bLegal = false;

            if (!m_bLegal) return;

            Command innerCommand = checkedCommand.GetParams()[3];
            if (innerCommand.GetType() != CommandTypes::ASSIGNMENT)
                m_bLegal = false;

            if (!m_bLegal) return;

            Command innerCommand = checkedCommand.GetParams()[4];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression()) m_bLegal = false;

            if (!m_bLegal) return;

            Command innerCommand = checkedCommand.GetParams()[6];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression()) m_bLegal = false;

            if (!m_bLegal) return;

            Command innerCommand = checkedCommand.GetParams()[8];
            ReformCommand(innerCommand);
            if (!innerCommand.IsExpression()) m_bLegal = false;
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

        if (m_bLegal) checkedCommand.SetType(CommandTypes::EXPRESSION);
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

        if (m_bLegal) checkedCommand.SetType(CommandTypes::EXPRESSION);
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_DECLARATION)
    {
        int iParamIndex = 0;
        bool bExpectingExpression = true;

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
        
        if (m_bLegal) checkedCommand.SetType(CommandTypes::EXPRESSION); 
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        int iParamIndex = 0;
        bool bExpectingExpression = true;

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
        
        if (m_bLegal) checkedCommand.SetType(CommandTypes::EXPRESSION); 
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
    else if (/*Here should be all the non finished types*/)
    {
        m_bLegal = false;
        checkedCommand.SetType(CommandTypes::UNKNOWN);
    }
}
