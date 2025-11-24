#include "parser/commandLine.h"

CommandLine::CommandLine(const EntityLogger& entityLogger): 
m_entityLogger(entityLogger), m_bFinished(false), m_bLegal(false)
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
            else{
                m_uLineIndex++;
                m_line.push_back(Command(m_entityLogger));
            }
        }
    }

    if (m_bFinished)
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
