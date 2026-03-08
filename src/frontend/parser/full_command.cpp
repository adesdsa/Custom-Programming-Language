#include "includes/frontend/parser/full_command.h"

unsigned int FullCommand::m_uBlockStartCnt = 0;
unsigned int FullCommand::m_uBlockCloseCnt = 0;

FullCommand::FullCommand(const EntityLogger& entityLogger, bool importLegal) : 
    m_entityLogger(entityLogger), 
    m_bFinished(false), 
    m_bLegal(true),
    m_importLegal(importLegal),
    m_uLineIndex(0)
{}

FullCommand::FullCommand(const FullCommand& other):
    m_entityLogger(other.m_entityLogger),
    m_bFinished(other.m_bFinished),
    m_bLegal(other.m_bLegal),
    m_line(other.m_line),
    m_importLegal(other.m_importLegal),
    m_uLineIndex(other.m_uLineIndex),
    m_errors(other.m_errors)
{}

FullCommand& FullCommand::operator=(const FullCommand& other)
{
    if (this == &other)
    {
        return *this;
    }
    this->m_bFinished   = other.m_bFinished;
    this->m_bLegal      = other.m_bLegal;
    this->m_line        = other.m_line;
    this->m_uLineIndex  = other.m_uLineIndex;
    this->m_errors      = other.m_errors;
    return *this;
}

bool FullCommand::HasCommands() const noexcept
{
    return this->m_line.size() > 0;
}

bool FullCommand::IsFinished() const
{
    return this->m_bFinished;
}

bool FullCommand::IsLegal() const
{
    return this->m_bLegal;
}

const std::vector<Command>& FullCommand::GetLine() const
{
    return this->m_line;
}

const std::vector<CompileTimeError>& FullCommand::GetErrors() const
{
    return this->m_errors;
}

bool FullCommand::AddToken(Token& currToken, Token& nextToken)
{
    if (!this->m_bFinished)
    {
        // Checking if new Command is needed in the Line.
        if (this->m_uLineIndex == this->m_line.size())
        {
            this->m_line.push_back(Command());
        }

        if (this->m_line[this->m_uLineIndex].Append(currToken, nextToken))
        {
            this->m_entityLogger.Info("Finished creating a new Command");

            if (this->m_importLegal && 
                (this->m_line[this->m_uLineIndex].GetType() != CommandTypes::IMPORT_STATEMENT && 
                this->m_line[this->m_uLineIndex].GetType() != CommandTypes::UNKNOWN)
            )
            {
                this->m_importLegal = false;
            }
            // Checking if the line is finished.
            if (this->m_line[this->m_uLineIndex].GetType() == CommandTypes::END_OF_COMMAND)
            {
                this->m_bFinished = true;
            }
            else if (this->m_line[this->m_uLineIndex].GetType() == CommandTypes::IMPORT_STATEMENT)
            {
                if (this->m_importLegal)
                {
                    this->m_bFinished = true;
                }
                else
                {
                    FoundIllegal(this->m_line[this->m_uLineIndex],
                        "Import found at the middle of the code",
                        "Move the Import to the top of the code",
                        0
                    );
                    this->m_bFinished = true;
                }
            }
            else if (this->m_line[this->m_uLineIndex].GetType() == CommandTypes::BLOCK_START)
            {
                m_uBlockStartCnt++;
                this->m_bFinished = true;
            }
            else if (this->m_line[this->m_uLineIndex].GetType() == CommandTypes::BLOCK_END)
            {
                m_uBlockCloseCnt++;
                this->m_bFinished = true;

                // If we somehow have more closes than starts it's an error.
                if (m_uBlockCloseCnt > m_uBlockStartCnt)
                {
                    CompileTimeError error(ErrorTypes::SYNTAX_ERROR,
                        "Unexpected '}' — no matching opening block",
                        this->m_line[this->m_uLineIndex][0].GetLine(),
                        this->m_line[this->m_uLineIndex][0].GetColumn(),
                        "Check for a missing '{' or an extra '}'");
                    this->m_errors.push_back(error);
                }
                else if (m_uBlockCloseCnt == m_uBlockStartCnt)
                {
                    m_uBlockCloseCnt = 0;
                    m_uBlockStartCnt = 0;
                }
            }
            else if (this->m_line[this->m_uLineIndex].GetType() == CommandTypes::EOF_TOKEN)
            {
                this->m_bFinished = true;

                // In case not all code blocks were closed.
                if (m_uBlockStartCnt != m_uBlockCloseCnt)
                {
                    CompileTimeError error(ErrorTypes::SYNTAX_ERROR,
                        "Not all code blocks were closed at the end of the program",
                        this->m_line[this->m_uLineIndex][0].GetLine(),
                        this->m_line[this->m_uLineIndex][0].GetColumn(),
                        "Check for missing '}'");
                    this->m_errors.push_back(error);
                }

                // In case EOF reached before the last line was finished.
                if (this->m_line.size() != 0)
                {
                    CompileTimeError error(ErrorTypes::SYNTAX_ERROR,
                        "Unexpected end of file — code is incomplete",
                        this->m_line[0][0].GetLine(), this->m_line[0][0].GetColumn(),
                        "Possibly missing ';' at end of statement or '}' to close a block");
                    this->m_errors.push_back(error);
                }
            }
            // If we have an UNKNOWN type in the Command it's definitely illegal.
            else if (this->m_line[this->m_uLineIndex].GetType() == CommandTypes::UNKNOWN)
            {
                this->m_errors.push_back(this->m_line[this->m_uLineIndex].GetErrors()[0]);
                this->m_bLegal = false;
            }
            else 
            {
                // In any other case the Command is a normal
                // piece of code that needs to be reformed.
                this->m_entityLogger.Info("Started reforming the new Command");
                this->ReformCommand(this->m_line[this->m_uLineIndex]);
                this->m_entityLogger.Info("Finished reforming the new Command");
            }
            this->m_uLineIndex++;
        }
    }

    if (this->m_bFinished)
    {
        // If the line was found to be illegal until
        // now, no need to check if the line sequence is legal.
        if (this->m_bLegal)
        {
            this->CheckLine();
        }
        return true;
    }
    return false;
}

void FullCommand::CheckLine()
{
    this->m_entityLogger.Info("Checking the legality of the FullCommand");
    bool bLegalLine = false;

    try
    {
        if (this->m_line.at(0).GetType() == CommandTypes::IDENTIFIER ||
            this->m_line.at(0).GetType() == CommandTypes::ARRAY_ACCESS ||
            this->m_line.at(0).GetType() == CommandTypes::ARRAY_SLICE)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::ASSIGNMENT)
            {
                if (this->m_line.at(2).IsExpression())
                {
                    if (this->m_line.at(3).GetType() == CommandTypes::END_OF_COMMAND)
                    {
                        bLegalLine = true;
                    }
                }
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::IMPORT_STATEMENT)
        {
            bLegalLine = true;
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::ELSE)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::BLOCK_START)
            {
                bLegalLine = true;
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::RETURN)
        {
            if (this->m_line.size() == 2)
            {
                // Empty return will be translated as return void.
                Command tempCommand;
                tempCommand.SetType(CommandTypes::VOID_COMMAND);
                tempCommand.SetValue(Token("void",
                    TokenTypes::KEYWORD,
                    this->m_line.at(0)[0].GetLine(),
                    this->m_line.at(0)[0].GetColumn()));
                this->m_line.insert(this->m_line.begin() + 1, tempCommand);
                bLegalLine = true;
            }
            if (this->m_line.at(1).IsExpression() ||
                this->m_line.at(1).GetType() == CommandTypes::VOID_COMMAND)
            {
                if (this->m_line.at(2).GetType() == CommandTypes::END_OF_COMMAND)
                {
                    bLegalLine = true;
                }
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::VAR_DECLARATION)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::END_OF_COMMAND)
            {
                bLegalLine = true;
            }
            else if (this->m_line.at(1).GetType() == CommandTypes::ASSIGNMENT)
            {
                if (this->m_line.at(2).IsExpression())
                {
                    if (this->m_line.at(3).GetType() == CommandTypes::END_OF_COMMAND)
                    {
                        bLegalLine = true;
                    }
                }
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::CONTROL_STATEMENT)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::END_OF_COMMAND)
            {
                bLegalLine = true;
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::ELIF ||
                 this->m_line.at(0).GetType() == CommandTypes::IF ||
                 this->m_line.at(0).GetType() == CommandTypes::FOR ||
                 this->m_line.at(0).GetType() == CommandTypes::WHILE)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::BLOCK_START)
            {
                bLegalLine = true;
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::SIZEOF)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::END_OF_COMMAND)
            {
                bLegalLine = true;
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::FINISHED_FUNC_DECLARATION)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::BLOCK_START)
            {
                this->m_line.at(0).SetType(CommandTypes::FINISHED_FUNC_DEFINITION);
                bLegalLine = true;
            }
            else if (this->m_line.at(1).GetType() == CommandTypes::END_OF_COMMAND)
            {
                bLegalLine = true;
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::FINISHED_FUNC_CALL)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::END_OF_COMMAND)
            {
                bLegalLine = true;
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::ARRAY_DECLARATION)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::END_OF_COMMAND)
            {
                bLegalLine = true;
            }
            else if (this->m_line.at(1).GetType() == CommandTypes::ASSIGNMENT)
            {
                if (this->m_line.at(2).GetType() == CommandTypes::ARRAY_VALUE)
                {
                    if (this->m_line.at(3).GetType() == CommandTypes::END_OF_COMMAND)
                    {
                        bLegalLine = true;
                    }
                }
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::PASS)
        {
            if (this->m_line.at(1).GetType() == CommandTypes::END_OF_COMMAND)
            {
                bLegalLine = true;
            }
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::BLOCK_END)
        {
            bLegalLine = true;
        }
        else if (this->m_line.at(0).GetType() == CommandTypes::EOF_TOKEN)
        {
            bLegalLine = (m_uBlockStartCnt == m_uBlockCloseCnt);
        }
    }
    catch (const std::out_of_range&)
    {
        bLegalLine = false;
    }

    if (!bLegalLine)
    {
        this->m_entityLogger.Info("Found an illegal FullCommand");
        this->m_bLegal = false;
        CompileTimeError error(ErrorTypes::SYNTAX_ERROR,
            "Invalid syntax — could not parse this line",
            this->m_line[0][0].GetLine(), this->m_line[0][0].GetColumn(),
            "Check for typos, missing operators, or incorrect statement structure");
        this->m_errors.push_back(error);
    }
    else
    {
        this->m_entityLogger.Info("Found a legal FullCommand");
    }
}

void FullCommand::ReformCommand(Command& checkedCommand)
{
    if (checkedCommand.GetType() == CommandTypes::ELIF)
    {
        this->HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::IF)
    {
        this->HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::WHILE)
    {
        this->HandleSingleParamKeyword(checkedCommand);
    }
    else if (checkedCommand.GetType() == CommandTypes::IMPORT_STATEMENT)
    {
        for(auto imports : checkedCommand.GetParams())
        {
            if (imports.GetType() != CommandTypes::IDENTIFIER)
            {
                FoundIllegal(imports,
                    "Found illegal command isnside import",
                    "",
                    0
                );
            }
        }
        // wildcard import (import * from "...") has zero params — valid
    }
    else if (checkedCommand.GetType() == CommandTypes::SIZEOF)
    {
        if (this->CheckConstSize(checkedCommand, 4))
        {
            return;
        }
        else if (this->CheckParantheses(checkedCommand))
        {
            return;
        }
        else if (checkedCommand.GetParams()[0].GetType() == CommandTypes::VAR_TYPE)
        {
            checkedCommand.SetExpression(true);
            return;
        }
        else
        {
            this->CheckParamExpectedExpression(checkedCommand, 0);
        }

        if (this->m_bLegal)
        {
            checkedCommand.SetExpression(true);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::FOR)
    {
        if (checkedCommand.GetTokens().size() != 12)
        {
            checkedCommand.SetType(CommandTypes::UNKNOWN);
        }
        else if (checkedCommand.GetParanthesesCounter() != 0)
        {
            checkedCommand.SetType(CommandTypes::UNKNOWN);
        }
        else 
        {
            if (!this->CheckType(checkedCommand[2], TokenTypes::HAS_PARAM) ||
                !this->CheckType(checkedCommand[3], TokenTypes::HAS_PARAM) ||
                !this->CheckType(checkedCommand[4], TokenTypes::HAS_PARAM) ||
                !this->CheckValue(checkedCommand[5], ";") ||
                !this->CheckType(checkedCommand[6], TokenTypes::HAS_PARAM) ||
                !this->CheckValue(checkedCommand[7], ";") ||
                !this->CheckType(checkedCommand[8], TokenTypes::HAS_PARAM) ||
                !this->CheckType(checkedCommand[9], TokenTypes::HAS_PARAM) ||
                !this->CheckType(checkedCommand[10], TokenTypes::HAS_PARAM))
            {
                checkedCommand.SetType(CommandTypes::UNKNOWN);
            }

            if (this->CheckParamExpectedTypes(checkedCommand, 0,
                {CommandTypes::VAR_DECLARATION, CommandTypes::IDENTIFIER}))
            {
                return;
            }
            if (this->CheckParamExpectedTypes(checkedCommand, 1,
                {CommandTypes::ASSIGNMENT}))
            {
                return;
            }
            if (this->CheckParamExpectedExpression(checkedCommand, 2))
            {
                return;
            }
            if (this->CheckParamExpectedExpression(checkedCommand, 3))
            {
                return;
            }
            if (this->CheckParamExpectedTypes(checkedCommand, 4,
                {CommandTypes::IDENTIFIER}))
            {
                return;
            }
            if (this->CheckParamExpectedTypes(checkedCommand, 5,
                {CommandTypes::ASSIGNMENT}))
            {
                return;
            }
            if (this->CheckParamExpectedExpression(checkedCommand, 6))
            {
                return;
            }
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_VALUE)
    {
        int iParamIndex = 0;

        if (this->CheckParantheses(checkedCommand))
        {
            return;
        }
        for (size_t i = 0; i < checkedCommand.GetTokens().size(); i++)
        {
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
            {
                this->CheckParamExpectedExpression(checkedCommand, iParamIndex);
                iParamIndex++;
            }
        }

        if (this->m_bLegal) 
        {
            checkedCommand.SetType(CommandTypes::EXPRESSION);
            checkedCommand.SetExpression(true);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::EXPRESSION_OPERATOR)
    {
        int iParamIndex = 0;

        if (this->CheckParantheses(checkedCommand))
        {
            return;
        }
        for (size_t i = 0; i < checkedCommand.GetTokens().size(); i++)
        {
            if (checkedCommand[i].GetType() == TokenTypes::HAS_PARAM)
            {
                this->CheckParamExpectedExpression(checkedCommand, iParamIndex);
                iParamIndex++;
            }
        }

        if (this->CheckLastExpressionOperator(checkedCommand))
        {
            return;
        }

        if (this->m_bLegal) 
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
        {
            if (checkedCommand.GetParams()[0].GetType() == CommandTypes::VOID_COMMAND)
            {
                return;
            }
        }

        for (size_t i = 2; i + 3 < checkedCommand.GetTokens().size() &&
            this->m_bLegal; i++)
        {
            if (bExpectingExpression)
            {
                this->CheckParamExpectedTypes(checkedCommand, iParamIndex, 
                    {CommandTypes::VAR_DECLARATION});
                iParamIndex++;
            }
            else if (!bExpectingExpression && 
                     this->CheckValue(checkedCommand[i], "not"))
            {
                continue;
            }
            else if (!this->CheckValue(checkedCommand[i], ","))
            {
                this->FoundIllegal(checkedCommand,
                    "Expected to get a parameter seperator",
                    "Check if ',' was forgotten between two parameters", 
                    i);
            }
            bExpectingExpression = !bExpectingExpression;
        }

        this->CheckLastFuncParamComa(checkedCommand, 3);
    }
    else if (checkedCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        this->HandleFuncLikeExpressions(checkedCommand, 2);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_ACCESS)
    {
        this->HandleSingleParamKeyword(checkedCommand);
        if (this->m_bLegal)
        {
            checkedCommand.SetExpression(true);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_SLICE)
    {
        if (checkedCommand.GetParams().size() == 2)
        {
            this->CheckParamExpectedExpression(checkedCommand, 0);
            this->CheckParamExpectedExpression(checkedCommand, 1);
        }
        if (this->m_bLegal)
        {
            checkedCommand.SetExpression(true);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_DECLARATION)
    {
        if (this->CheckParantheses(checkedCommand))
        {
            return;
        }
        else if (checkedCommand.GetParams().size() > 1) 
        {
            return this->FoundIllegal(checkedCommand,
                "Too many args for an array",
                "Try removing the extra args",
                0);
        }
        else
        {
            this->CheckParamExpectedExpression(checkedCommand, 0);
        }
    }
    else if (checkedCommand.GetType() == CommandTypes::IDENTIFIER_VALUE)
    {
        checkedCommand.SetExpression(true);
    }
    else if (checkedCommand.GetType() == CommandTypes::ARRAY_VALUE)
    {
        this->HandleFuncLikeExpressions(checkedCommand, 1);
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
        this->FoundIllegal(
            checkedCommand,
            "Found an unfinished command",
            "Try finishing the command in the line specified",
            0);
    }
}

bool FullCommand::CheckConstSize(Command& checkedCommand, int iSize)
{
    if (checkedCommand.GetTokens().size() != iSize) 
    {
        this->FoundIllegal(
            checkedCommand,
            "'" + std::string(checkedCommand.GetType()) +
            "' statement has wrong number of tokens (expected " +
            std::to_string(iSize) +
            ", got " +
            std::to_string(checkedCommand.GetTokens().size()) + ")",
            "Check for missing or extra tokens in this statement", 
            0);
        return true;
    }
    return false;
}

bool FullCommand::CheckParantheses(Command& checkedCommand)
{
    if (checkedCommand.GetParanthesesCounter() != 0)
    {
        this->FoundIllegal(
            checkedCommand,
            "Unmatched parentheses — " +
            std::to_string(checkedCommand.GetParanthesesCounter())
            + " opening '(' never closed",
            "Check for missing closing ')'",
            0);
        return true;
    }
    return false;
}

bool FullCommand::CheckParamExpectedExpression(
    Command& checkedCommand, int iParamIndex)
{
    Command& innerCommand = checkedCommand.GetParams()[iParamIndex];
    this->ReformCommand(checkedCommand.GetParams()[iParamIndex]);
    if (!innerCommand.IsExpression())
    {
        this->FoundIllegal(
            checkedCommand,
            "Expected an expression at (" +
            std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetLine()) +
            "," +
            std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetColumn()) +
            ") but found '" + std::string(checkedCommand.GetType()) + "' instead",
            "Provide a valid expression here",
            0);
        return true;
    }
    return false;
}

bool FullCommand::CheckParamExpectedTypes(
    Command& checkedCommand, int iParamIndex, 
    std::vector<std::string_view> types)
{
    Command& innerCommand = checkedCommand.GetParams()[iParamIndex];
    bool bFoundCorrect = false;

    for (size_t i = 0; i < types.size(); i++)
    {
        if (innerCommand.GetType() == types[i])
        {
            bFoundCorrect = true;
        }
    }

    if (!bFoundCorrect)
    {
        std::string sTypes = std::string(types[0]);
        for (size_t i = 1; i < types.size(); i++)
        {
            sTypes += ", " + std::string(types[i]);
        }
        this->FoundIllegal(
            checkedCommand,
            "Expected " + sTypes +
            " at (" +
            std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetLine()) +
            "," +
            std::to_string(checkedCommand.GetParams()[iParamIndex][0].GetColumn()) +
            ") but found '" + std::string(innerCommand.GetType()) + "'",
            "Check for the correct token type here",
            0);
        return true;
    }

    return false;
}

bool FullCommand::CheckLastFuncParamComa(
    Command& checkedCommand, int iBackSteps)
{
    if (this->CheckValue(checkedCommand[checkedCommand.GetTokens().size()
        - iBackSteps], ","))
    {
        this->FoundIllegal(
            checkedCommand,
            "Trailing ',' in parameter list — expected another argument after ','",
            "Add the missing argument or remove the trailing ','",
            checkedCommand.GetTokens().size() - iBackSteps);
        return true;
    }
    return false;
}

void FullCommand::HandleSingleParamKeyword(Command& checkedCommand)
{
    if (this->CheckConstSize(checkedCommand, 4))
    {
        return;
    }
    else if (this->CheckParantheses(checkedCommand))
    {
        return;
    }
    else
    {
        this->CheckParamExpectedExpression(checkedCommand, 0);
    }
}

void FullCommand::HandleFuncLikeExpressions(
    Command& checkedCommand, int iStartingIndex)
{
    int iParamIndex = 0;
    bool bExpectingExpression = true;

    if (checkedCommand.GetTokens().size() == 4)
    {
        if (checkedCommand.GetParams()[0].GetType() == CommandTypes::VOID_COMMAND)
        {
            checkedCommand.SetExpression(true);
            return;
        }
    }

    for (size_t i = iStartingIndex; i + 1 < checkedCommand.GetTokens().size()
        && this->m_bLegal; i++)
    {
        if (bExpectingExpression)
        {
            this->CheckParamExpectedExpression(checkedCommand, iParamIndex);
            iParamIndex++;
        }
        else if (!bExpectingExpression && 
                 (this->CheckValue(checkedCommand[i], "not") ||
                  this->CheckValue(checkedCommand[i], "*") ||
                  this->CheckValue(checkedCommand[i], "&")))
        {
            continue;
        }
        else if (!this->CheckValue(checkedCommand[i], ","))
        {
            this->FoundIllegal(checkedCommand,
                "Missing ',' between function arguments",
                "Add ',' to separate the arguments",
                i);
        }
        bExpectingExpression = !bExpectingExpression;
    }

    this->CheckLastFuncParamComa(checkedCommand, 2);
    
    if (this->m_bLegal)
    {
        checkedCommand.SetExpression(true);
    }
}

inline bool FullCommand::CheckLastExpressionOperator(Command& checkedCommand)
{
    const Token lastToken = checkedCommand[checkedCommand.GetTokens().size() - 2];
    if ((lastToken.GetType() == TokenTypes::OPERATOR ||
         lastToken.GetType() == TokenTypes::KEYWORD)
        && lastToken.GetValue() != ")")
    {
        this->FoundIllegal(
            checkedCommand,
            "Expression ends with an operator — expected a value after it",
            "Add a value after the operator or remove the trailing operator",
            0);
        return true;
    }
    return false;
}

inline bool FullCommand::CheckType(
    const Token& token, std::string_view sCompare) const
{
    return token.GetType() == sCompare;
}

inline bool FullCommand::CheckValue(
    const Token& token, std::string sCompare) const
{
    return token.GetValue() == sCompare;
}

void FullCommand::FoundIllegal(
    Command& command,
    std::string sErrorContent,
    std::string sHint,
    size_t uTokenIndex)
{
    // In case there are errors in the params.
    if (command.GetErrors().size() > 0)
    {
        for (size_t i = 0; i < command.GetErrors().size(); i++)
        {
            this->m_errors.push_back(command.GetErrors()[i]);
        }
    }
    CompileTimeError error(
        ErrorTypes::SYNTAX_ERROR,
        sErrorContent,
        command[uTokenIndex].GetLine(),
        command[uTokenIndex].GetColumn(),
        sHint);
    this->m_errors.push_back(error);
    this->m_bLegal = false;
    command.SetType(CommandTypes::UNKNOWN);
    command.SetExpression(false);
}
