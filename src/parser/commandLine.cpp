#include "parser/commandLine.h"

unsigned int CommandLine::m_uBlockStartCnt = 0;
unsigned int CommandLine::m_uBlockCloseCnt = 0;

CommandLine::CommandLine(const EntityLogger& entityLogger): 
m_entityLogger(entityLogger), m_bFinished(false), m_bLegal(true),
m_uLineIndex(0)
{
}

CommandLine::CommandLine(const CommandLine& other):
    m_entityLogger(other.m_entityLogger),
    m_bFinished(other.m_bFinished),
    m_bLegal(other.m_bLegal),
    m_line(other.m_line),
    m_uLineIndex(other.m_uLineIndex),
    m_errors(other.m_errors)
{
}

CommandLine& CommandLine::operator=(const CommandLine& other) {
    if (this == &other)
        return *this;
    m_bFinished   = other.m_bFinished;
    m_bLegal      = other.m_bLegal;
    m_line        = other.m_line;
    m_uLineIndex  = other.m_uLineIndex;
    m_errors      = other.m_errors;
    return *this;
}

bool CommandLine::HasCommands() const noexcept {
    return this->m_line.size() > 0;
}


bool CommandLine::IsFinished() const {
    return m_bFinished;
}

bool CommandLine::IsLegal() const {
    return m_bLegal;
}

const std::vector<Command>& CommandLine::GetLine() const {
    return m_line;
}

const std::vector<CompileTimeError>& CommandLine::GetErrors() const {
    return m_errors;
}

bool CommandLine::AddToken(Token& currToken, Token& nextToken)
{
    // todo: check if the Errors work correctly with the parameters.
    // todo: add logs and docs
    if (!m_bFinished)
    {
        if (m_uLineIndex == m_line.size()) m_line.push_back(Command());
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
                    CompileTimeError error(ErrorTypes::SYNTAX_ERROR, "Tried closing a block without being in one", m_line[m_uLineIndex][0].GetLine(), m_line[m_uLineIndex][0].GetColumn(), "");
                    m_errors.push_back(error);
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
                    CompileTimeError error(ErrorTypes::SYNTAX_ERROR, "Not all code blocks were closed at the end of the program", m_line[m_uLineIndex][0].GetLine(), m_line[m_uLineIndex][0].GetColumn(), "");
                    m_errors.push_back(error);
                }
                if (m_line.size() != 0)
                {
                    CompileTimeError error(ErrorTypes::SYNTAX_ERROR, "EOF detected before code was finished", m_line[0][0].GetLine(), m_line[0][0].GetColumn(), "Probably forgot to end the line or the code block-try adding ; or }");
                    m_errors.push_back(error);
                }
            }
            // If we have and UNKOWN type in the Command it's definitly illegal.
            else if (m_line[m_uLineIndex].GetType() == CommandTypes::UNKNOWN)
            {
                m_errors.push_back(m_line[m_uLineIndex].GetErrors()[0]);
                m_bLegal = false;
            }
            else {
                // Reforming the command incase it's needed and continuing to the next Command.
                ReformCommand(m_line[m_uLineIndex]);
            }
            m_uLineIndex++;
        }
    }

    if (m_bFinished) // If it's illegal no reason to check.
    {
        if (m_bLegal)
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
        CompileTimeError error(ErrorTypes::SYNTAX_ERROR, "Couldn't parse the commands into a legal code line", m_line[0][0].GetLine(), m_line[0][0].GetColumn(), "");
        m_errors.push_back(error);
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
        if (CheckConstSize(checkedCommand, 4)) return;
        else if (CheckParantheses(checkedCommand)) return;
        else if (checkedCommand.GetParams()[0].GetType() == CommandTypes::VAR_TYPE) return;
        else CheckParamExpectedExpression(checkedCommand, 0);
    }
    else if (checkedCommand.GetType() == CommandTypes::FOR)
    {
        if (checkedCommand.GetTokens().size() != 12) checkedCommand.SetType(CommandTypes::UNKNOWN);
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
            !CheckType(checkedCommand[8], TokenTypes::HAS_PARAM) ||
            !CheckType(checkedCommand[9], TokenTypes::HAS_PARAM) ||
            !CheckType(checkedCommand[10], TokenTypes::HAS_PARAM))
                checkedCommand.SetType(CommandTypes::UNKNOWN);


            if (CheckParamExpectedTypes(checkedCommand, 0, {CommandTypes::VAR_DECLARATION, CommandTypes::IDENTIFIER})) return;

            if (CheckParamExpectedTypes(checkedCommand, 1, {CommandTypes::ASSIGNMENT})) return;

            if (CheckParamExpectedExpression(checkedCommand, 2)) return;

            if (CheckParamExpectedExpression(checkedCommand, 3)) return;

            if (CheckParamExpectedTypes(checkedCommand, 4, {CommandTypes::IDENTIFIER})) return;

            if (CheckParamExpectedTypes(checkedCommand, 5, {CommandTypes::ASSIGNMENT})) return;

            if (CheckParamExpectedExpression(checkedCommand, 6)) return;
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_VALUE)
    {
        int iParamIndex = 0;

        if (CheckParantheses(checkedCommand)) return;
        for (int i = 0; i < checkedCommand.GetTokens().size(); i++)
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
            {
                CheckParamExpectedExpression(checkedCommand, iParamIndex);
                iParamIndex++;
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

        if (CheckParantheses(checkedCommand)) return;
        for (int i = 0; i < checkedCommand.GetTokens().size(); i++)
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
            {
                CheckParamExpectedExpression(checkedCommand, iParamIndex);
                iParamIndex++;
            }

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

        if (checkedCommand.GetTokens().size() == 6)
            if (checkedCommand.GetParams()[0].GetType() == CommandTypes::VOID_COMMAND)
                return;

        for (int i = 2; i < checkedCommand.GetTokens().size() - 3 && m_bLegal; i++)
        {
            if (bExpectingExpression)
            {
                CheckParamExpectedTypes(checkedCommand, iParamIndex, {CommandTypes::VAR_DECLARATION});
                iParamIndex++;
            }
            else if (!bExpectingExpression && (CheckValue(checkedCommand[i], "not") || CheckValue(checkedCommand[i], "*") || CheckValue(checkedCommand[i], "&"))) continue;
            else if (!CheckValue(checkedCommand[i], ",")) FoundIllegal(checkedCommand, "Expected to get a parameter seperator", "Check if ',' was forgotten between two parameters", i);
            bExpectingExpression = !bExpectingExpression;
        }

        CheckLastFuncParamComa(checkedCommand, 3);
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        HandleFuncLikeExpressions(checkedCommand, 2);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_ACCESS)
    {
        HandleSingleParamKeyword(checkedCommand);
        if (m_bLegal) checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_DECLARATION)
    {
        if (CheckConstSize(checkedCommand, 7)) return;
        else if (CheckParantheses(checkedCommand)) return;
        else CheckParamExpectedExpression(checkedCommand, 0);
    }
    else if (checkedCommand.GetType() == CommandTypes::IDENTIFIER)
    {
        checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_VALUE)
    {
        HandleFuncLikeExpressions(checkedCommand, 1);
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
        FoundIllegal(checkedCommand, "Found an unfinished command", "Try finishing the command in the line specified", 0);
    }
}

bool CommandLine::CheckConstSize(Command& checkedCommand, int iSize)
{
    if (checkedCommand.GetTokens().size() != iSize) 
    {
        FoundIllegal(checkedCommand, "Expected the command (" + std::string(checkedCommand.GetType()) + ") to have the next size " + std::to_string(iSize) + " but the size is " + std::to_string(checkedCommand.GetTokens().size()), "", 0);
        return true;
    }
    return false;
}

bool CommandLine::CheckParantheses(Command& checkedCommand)
{
    if (checkedCommand.GetParanthesesCounter() != 0)
    {
        FoundIllegal(checkedCommand, "The command has too many open (and unclosed) parantheses", "There are " + std::to_string(checkedCommand.GetParanthesesCounter()) + " open parantheses", 0);
        return true;
    }
    return false;
}

bool CommandLine::CheckParamExpectedExpression(Command& checkedCommand, int iParamIndex)
{
    Command& innerCommand = checkedCommand.GetParams()[iParamIndex];
    ReformCommand(checkedCommand.GetParams()[iParamIndex]);
    if (!innerCommand.IsExpression())
    {
        FoundIllegal(checkedCommand, "Expected the command here: [" + std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetLine()) + ", " + std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetColumn()) + "] to be an expression but it's " + std::string(checkedCommand.GetType()), "", 0);
        return true;
    }
    return false;
}

bool CommandLine::CheckParamExpectedTypes(Command& checkedCommand, int iParamIndex, std::vector<std::string_view> types)
{
    Command& innerCommand = checkedCommand.GetParams()[iParamIndex];
    bool bFoundCorrect = false;
    for (int i = 0; i < types.size(); i++)
        if (innerCommand.GetType() == types[i]) 
            bFoundCorrect = true;

    if (!bFoundCorrect)
    {
        std::string sTypes = std::string(types[0]);
        for (int i = 1; i < types.size(); i++)
        {
            sTypes += ", " + std::string(types[i]);
        }
        FoundIllegal(checkedCommand, "The type of the command at [" + std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetLine()) + ", " + std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetColumn()) + "] should be: " + sTypes + ". But the type is: " + std::string(innerCommand.GetType()), "", 0);
        return true;
    }

    return false;
}

bool CommandLine::CheckLastFuncParamComa(Command& checkedCommand, int iBackSteps)
{
    if (CheckValue(checkedCommand[checkedCommand.GetTokens().size() - iBackSteps], ","))
    {
        FoundIllegal(checkedCommand, "The last thing in the parameters was a seperator ',' but the parameters should end with a parameter and not a seperator", "most likely forgot to add a parameter or removal of the seperator is needed", checkedCommand.GetTokens().size() - iBackSteps);
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

void CommandLine::HandleFuncLikeExpressions(Command& checkedCommand, int iStartingIndex)
{
    int iParamIndex = 0;
    bool bExpectingExpression = true;

    if (checkedCommand.GetTokens().size() == 4)
    {
        if (CheckType(checkedCommand.GetParams()[0][0], CommandTypes::VOID_COMMAND))
        {
            checkedCommand.SetExpression(true);
            return;
        }
    }

    for (int i = iStartingIndex; i < checkedCommand.GetTokens().size() - 1 && m_bLegal; i++)
    {
        if (bExpectingExpression)
        {
            CheckParamExpectedExpression(checkedCommand, iParamIndex);
            iParamIndex++;
        }
        else if (!bExpectingExpression && (CheckValue(checkedCommand[i], "not") || CheckValue(checkedCommand[i], "*") || CheckValue(checkedCommand[i], "&"))) continue;
        else if (!CheckValue(checkedCommand[i], ",")) FoundIllegal(checkedCommand, "Expected to get a parameter seperator", "Check if ',' was forgotten between two parameters", i);
        bExpectingExpression = !bExpectingExpression;
    }

    CheckLastFuncParamComa(checkedCommand, 2);
    
    if (m_bLegal) checkedCommand.SetExpression(true);
}

inline bool CommandLine::CheckLastExpressionOperator(Command& checkedCommand)
{
    Token lastToken = checkedCommand[checkedCommand.GetTokens().size() - 1];
    if (lastToken.GetValue() != ")") 
    {
        FoundIllegal(checkedCommand, "The expression was finished with an operator which is illegal", "most likely forgot to add a value or removal of the operator is needed", 0);
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

void CommandLine::FoundIllegal(Command& command, std::string sErrorContent, std::string sHint, unsigned int uTokenIndex)
{
    CompileTimeError error(ErrorTypes::SYNTAX_ERROR, sErrorContent, command[uTokenIndex].GetLine(), command[uTokenIndex].GetColumn(), sHint);
    m_errors.push_back(error);
    m_bLegal = false;
    command.SetType(CommandTypes::UNKNOWN);
    command.SetExpression(false);
}
