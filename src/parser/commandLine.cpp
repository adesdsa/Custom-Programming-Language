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
        
    }
    else if (checkedCommand.GetType() == CommandTypes::FOR)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::WHILE)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_VALUE)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_OPERATOR)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_DECLARATION)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_ACCESS)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_DECLARATION)
    {
        
    }
    else if (checkedCommand.GetType() == CommandTypes::IDENTIFIER)
    {

    }
}
