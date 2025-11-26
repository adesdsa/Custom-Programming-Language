#include "parser_component/commandLine.h"

unsigned int CommandLine::m_uBlockStartCnt = 0;
unsigned int CommandLine::m_uBlockCloseCnt = 0;

CommandLine::CommandLine(const EntityLogger& entityLogger): 
m_entityLogger(entityLogger), m_bFinished(false), m_bLegal(true)
{
    m_line.push_back(Command());
}

bool CommandLine::IsFinished() const
{
    return m_bFinished;
}

bool CommandLine::IsLegal() const // todo: Change this to return errors
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
            // Checking if the line is finished.
            if (m_line[m_uLineIndex].GetType() == CommandTypes::END_OF_COMMAND)
                m_bFinished = true;
            else if (m_line[m_uLineIndex].GetType() == CommandTypes::BLOCK_START)
            {
                m_uBlockStartCnt++;
                m_bFinished = true;
            }
            else if (m_line[m_uLineIndex].GetType() == CommandTypes::BLOCK_END)
            {
                m_uBlockCloseCnt++;
                m_bFinished = true;
                if (m_uBlockCloseCnt > m_uBlockStartCnt) // If we somehow have more closes than starts it's an error.
                {
                    // here you should make an error that says that you closed a block without being in one.
                }
                else if (m_uBlockCloseCnt == m_uBlockStartCnt)
                {
                    m_uBlockCloseCnt = 0;
                    m_uBlockStartCnt = 0;
                }
            }
            else if (m_line[m_uLineIndex].GetType() == CommandTypes::EOF_TOKEN)
            {
                m_bFinished = true;
                if (m_uBlockStartCnt != m_uBlockCloseCnt)
                {
                    // todo: handle error that not all code blocks were closed
                }
                if (m_line.size() != 0)
                {
                    // todo: throw error
                }
            }
            // If we have and UNKOWN type in the Command it's definitly illegal.
            else if (m_line[m_uLineIndex].GetType() == CommandTypes::UNKNOWN)
                m_bLegal = false;
            else {
                // Reforming the command incase it's needed and continuing to the next Command.
                ReformCommand(m_line[m_uLineIndex]);
                m_uLineIndex++;
                m_line.push_back(Command());
            }
        }
    }

    if (m_bFinished && m_bLegal) // If it's illegal no reason to check.
    {
        CheckLine();
        return true;
    }
    return false;
}

void CommandLine::CheckLine()
{
    // In this function unline the other functions we check if something is 
    // legal, not if something breaks the legality so we use this flag.
    bool bLegalLine = false;

    //
    // The next code segment will work this way =>
    // It will check if Command1 Command2 and so on make a legal syntax command.
    // e.g. VAR_DECLARATION + ASSIGNMENT + EXPRESSION.
    // It does this by checking: is Command1 VAR_DECLARATION then is Command2 ASSIGNMENT and so on.
    //

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
    else if (m_line[0].GetType() == CommandTypes::SIZEOF)
    {
        if (m_line[1].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
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
        if (m_line[1].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
        else if (m_line[1].GetType() == CommandTypes::ASSIGNMENT)
            if (m_line[2].GetType() == CommandTypes::ARRAY_VALUE)
                if (m_line[3].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::PASS)
    {
        if (m_line[1].GetType() == CommandTypes::END_OF_COMMAND) bLegalLine = true;
    }
    else if (m_line[0].GetType() == CommandTypes::BLOCK_END) bLegalLine = true;
    else if (m_line[0].GetType() == CommandTypes::EOF_TOKEN)
        bLegalLine = m_uBlockStartCnt == m_uBlockCloseCnt;

    if (!bLegalLine) {
        m_bLegal = false;
        // todo: handle error
    }
}

void CommandLine::ReformCommand(Command& checkedCommand)
{
    if (checkedCommand.GetType() == CommandTypes::ELIF)
    {
        HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::IF)
    {
        HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::WHILE)
    {
        HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::SIZEOF)
    {
        HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::FOR)
    {
        if (checkedCommand.GetTokens().size() != 10) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else if (checkedCommand.GetParanthesesCounter() != 0) checkedCommand.SetType(CommandTypes::UNKNOWN);
        else 
        {
            // A for statement is made of the next Token's
            // for+(+VAR_DECLRATION/IDENTIFIER+ASSIGNMENT+EXPRESSION+;+EXPRESSION+;+EXPRESSION+)
            // Checking if one of the Token's types/values are wrong
            if (!CheckType(checkedCommand[2], TokenTypes::HAS_PARAM) ||
            !CheckType(checkedCommand[3], TokenTypes::HAS_PARAM) ||
            !CheckType(checkedCommand[4], TokenTypes::HAS_PARAM) ||
            !CheckValue(checkedCommand[5], ";") ||
            !CheckType(checkedCommand[6], TokenTypes::HAS_PARAM) ||
            !CheckValue(checkedCommand[7], ";") ||
            !CheckType(checkedCommand[8], TokenTypes::HAS_PARAM))
                checkedCommand.SetType(CommandTypes::UNKNOWN);


            if (CheckParamExpectedTypes(checkedCommand, 2, {CommandTypes::VAR_DECLARATION, CommandTypes::IDENTIFIER})) return;

            if (CheckParamExpectedTypes(checkedCommand, 3, {CommandTypes::ASSIGNMENT})) return;

            if (CheckParamExpectedExpression(checkedCommand, 4)) return;

            if (CheckParamExpectedExpression(checkedCommand, 6)) return;

            CheckParamExpectedExpression(checkedCommand, 8);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_VALUE)
    {
        int iParamIndex = 0;

        if (CheckParantheses(checkedCommand)) return;
        for (int i = 0; i < checkedCommand.GetTokens().size(); i++)
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
                CheckParamExpectedExpression(checkedCommand, iParamIndex);

        if (m_bLegal) 
        {
            checkedCommand.SetType(CommandTypes::EXPRESSION);
            checkedCommand.SetExpression(true);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_OPERATOR)
    {
        int iParamIndex = 0;

        if (CheckParantheses(checkedCommand)) return;
        for (int i = 0; i < checkedCommand.GetTokens().size(); i++)
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
                CheckParamExpectedExpression(checkedCommand, iParamIndex);

        if (CheckLastExpressionOperator(checkedCommand)) return;

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
            if (CheckType(checkedCommand.GetParams()[2][0], CommandTypes::VOID_COMMAND))
                return;

        for (int i = 2; i < checkedCommand.GetTokens().size() - 1 && m_bLegal; i++)
        {
            if (bExpectingExpression)
            {
                CheckParamExpectedTypes(checkedCommand, iParamIndex, {CommandTypes::VAR_DECLARATION});
                iParamIndex++;
            }
            else if (!CheckValue(checkedCommand[i], ",")) 
            {
                // todo: handle error
                FoundIllegal(checkedCommand);
            }
        }

        CheckLastFuncParamComa(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        HandleFuncLikeExpressions(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_ACCESS)
    {
        HandleSingleParamKeyword(checkedCommand);
        if (m_bLegal) checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_DECLARATION)
    {
        HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::IDENTIFIER)
    {
        checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_VALUE)
    {
        HandleFuncLikeExpressions(checkedCommand);
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
        FoundIllegal(checkedCommand);
    }
}

bool CommandLine::CheckConstSize(Command& checkedCommand, int iSize)
{
    if (checkedCommand.GetTokens().size() != iSize) 
    {
        // todo: handle error
        FoundIllegal(checkedCommand);
        return true;
    }
    return false;
}

bool CommandLine::CheckParantheses(Command& checkedCommand)
{
    if (checkedCommand.GetParanthesesCounter() != 0)
    {
        // todo: handle error
        FoundIllegal(checkedCommand);
        return true;
    }
    return false;
}

bool CommandLine::CheckParamExpectedExpression(Command& checkedCommand, int iParamIndex)
{
    Command innerCommand = checkedCommand.GetParams()[iParamIndex];
    ReformCommand(innerCommand);
    if (!innerCommand.IsExpression())
    {
        // todo: handle error
        FoundIllegal(checkedCommand);
        return true;
    }
    return false;
}

bool CommandLine::CheckParamExpectedTypes(Command& checkedCommand, int iParamIndex, std::vector<std::string_view> types)
{
    Command innerCommand = checkedCommand.GetParams()[iParamIndex];
    bool bFoundCorrect = false;
    for (int i = 0; i < types.size(); i++)
        if (innerCommand.GetType() == types[i]) 
            bFoundCorrect = true;

    if (!bFoundCorrect)
    {
        // todo: handle error
        FoundIllegal(checkedCommand);
        return true;
    }

    return false;
}

bool CommandLine::CheckLastFuncParamComa(Command& checkedCommand)
{
    if (CheckValue(checkedCommand[checkedCommand.GetTokens().size() - 2], ","))
    {
        // todo: handle error.
        FoundIllegal(checkedCommand);
        return true;
    }
    return false;
}

void CommandLine::HandleSingleParamKeyword(Command& checkedCommand)
{
    if (CheckConstSize(checkedCommand, 4)) return;
    else if (CheckParantheses(checkedCommand)) return;
    else CheckParamExpectedExpression(checkedCommand, 0);
}

void CommandLine::HandleFuncLikeExpressions(Command& checkedCommand)
{
    int iParamIndex = 0;
    bool bExpectingExpression = true;

    if (checkedCommand.GetTokens().size() == 4)
    {
        if (CheckType(checkedCommand.GetParams()[2][0], CommandTypes::VOID_COMMAND))
        {
            checkedCommand.SetExpression(true);
            return;
        }
    }

    for (int i = 2; i < checkedCommand.GetTokens().size() - 1 && m_bLegal; i++)
    {
        if (bExpectingExpression)
        {
            CheckParamExpectedExpression(checkedCommand, iParamIndex);
            iParamIndex++;
        }
        else if (!CheckValue(checkedCommand[i], ","))
        {
            // todo: handle error
            FoundIllegal(checkedCommand);
        }
    }

    CheckLastFuncParamComa(checkedCommand);
    
    if (m_bLegal) checkedCommand.SetExpression(true);
}

inline bool CommandLine::CheckLastExpressionOperator(Command& checkedCommand)
{
    Token lastToken = checkedCommand[checkedCommand.GetTokens().size() - 1];
    if (lastToken.GetValue() != ")") 
    {
        FoundIllegal(checkedCommand);
        return true;
    }
    return false;
}

inline bool CommandLine::CheckType(const Token& token, std::string_view sCompare) const{
    return token.GetType() == sCompare;
}

inline bool CommandLine::CheckValue(const Token& token, std::string sCompare) const{
    return token.GetValue() == sCompare;
}

bool CommandLine::IsType(const Token& token) const
{
    return token.GetValue() == "int" || token.GetValue() == "bool" || token.GetValue() == "double";
}

void CommandLine::FoundIllegal(Command& command)
{
    m_bLegal = false;
    command.SetType(CommandTypes::UNKNOWN);
    command.SetExpression(false);
}
