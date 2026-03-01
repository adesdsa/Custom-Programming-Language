#pragma once

#include "command.h"
#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/errors/error_models.h"


//  The class is used to create Commands and check if their
// sequence is legal from a syntax prespective.
//  Command is the smallest unit of code syntax speaking;
// For more info please check Command.h
//  Line = the sequence of the Commands.
class FullCommand{
public:
    FullCommand(const EntityLogger& entityLogger);
    FullCommand(const FullCommand& other);
    
    FullCommand& operator=(const FullCommand& other);

    // The func is used to add token to the line.
    // When the line is finished it will check it's legallity.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token that will be added.
    // @return: True-> Finished the line. False-> Otherwise.
    bool AddToken(Token& currToken, Token& nextToken);

    // Getters

    // Is The Line Finished.
    bool IsFinished() const;
    // Is The Line Legal.
    bool IsLegal() const;
    // Get The Line Itself.
    const std::vector<Command>& GetLine() const;
    // Checks if the command line isn't empty
    bool HasCommands() const noexcept;
    // Gets the errors that were found during the syntax checking of the line.
    const std::vector<CompileTimeError>& GetErrors() const;


private:
    const EntityLogger& m_entityLogger;
    bool m_bFinished;
    bool m_bLegal;
    std::vector<Command> m_line;
    size_t m_uLineIndex;
    static unsigned int m_uBlockStartCnt;
    static unsigned int m_uBlockCloseCnt;
    std::vector<CompileTimeError> m_errors;

    // The func is used to check if the line is legal syntaxly speaking.
    // It checks that the sequense of token1 + token2 and so on work together.
    void CheckLine();

    // The funcs is used to reform the Commands.
    // It takes a single Command and checks if it and it's 
    // parameters are legal.
    // If it found something illegal it will add an error.
    void ReformCommand(Command& checkedCommand);

    // The func checks that the size of the command is correct,
    // and handles it if not.
    // @param checkedCommand: The Command that is checked.
    // @param iSize: The size that the Command needs to be.
    // @return: True-> The size isnt what is expected.
    // False-> Otherwise.
    bool CheckConstSize(Command& checkedCommand, int iSize);

    // The func checks that the Command closed all the parantheses,
    // and handles it if not.
    // @param checkedCommand: The Command we check.
    // @return: True-> Not all Parantheses are closed.
    // False-> Otherwise.
    bool CheckParantheses(Command& checkedCommand);

    // The func checks that the parameter at the iParamIndex is an expression,
    // it does this by reforming the parameter, and handles it if not.
    // @param checkedCommand: The Command that has the parameter we check.
    // @param iParamIndex: The parameter index.
    // @return: True-> The parameter isnt an expression. False-> Otherwise.
    bool CheckParamExpectedExpression(
        Command& checkedCommand, int iParamIndex);

    // The func checks that the parameter at the index is one
    // of the given types, and handles it if not.
    // @param checkedCommand: The Command that has the parameter we check.
    // @param iParamIndex: The parameter index.
    // @param types: The expected types.
    // @return: True-> The parameter isnt one of the expected types.
    // False-> Otherwise.
    bool CheckParamExpectedTypes(
        Command& checkedCommand, int iParamIndex, std::vector<std::string_view> types);
    
    // The func checks if the last parameter is a comma (a seperator),
    // if it is that's illegal and it handles it.
    // It is supposed to be used in func like FullCommands.
    // @param checkedCommand: The Command that has the parameter we check. 
    // @param iBackSteps: The index we remove from the end.
    // @return: True-> The last parameter is a coma. False-> Otherwise.
    bool CheckLastFuncParamComa(Command& checkedCommand, int iBackSteps);

    // The func handles parameters with a single parameter e.g.:
    // if, elif, while.
    // Those keywords have a certain const preset.
    // @param checkedCommand: The Command that has the parameter we check.
    void HandleSingleParamKeyword(Command& checkedCommand);

    // The func handles function like expressions e.g.:
    // function calls and array_values.
    // Anything that is of the next preset: (expresion, expression,...)
    // @param checkedCommand: The Command that has the parameter we check. 
    // @param iStartingIndex: The index we start checking from.
    void HandleFuncLikeExpressions(
        Command& checkedCommand, int iStartingIndex);

    // The func is used to check if the last this
    // in the Command is an operator.
    // An expression cannot end with an operator.
    // @param checkedCommand: The Command that has the parameter we check.
    // @return: True-> The Command ends with an expression. False-> Otherwise.
    inline bool CheckLastExpressionOperator(Command& checkedCommand);

    // The func is used to check if the token is the correct type;
    // @param token: The Token we are checking.
    // @param sCompare: The TokenType we are comparing.
    // @return: True-> Token is the type. False-> Otherwise.
    inline bool CheckType(
        const Token& token, std::string_view sCompare) const;

    // The func is used to check if the token is the correct value;
    // @param token: The Token we are checking.
    // @param sCompare: The value we are comparing.
    // @return: True-> Token has the same value. False-> Otherwise.
    inline bool CheckValue(const Token& token, std::string sCompare) const;

    // The func handles the case where the Command is Illegal.
    // @param command: The Command that is illegal.
    // @param sErrorContent: 
    // The content of the error that will be added to the CompileTimeError.
    // @param sHint: The hint that will be added to the CompileTimeError.
    // @param uTokenIndex: The index of the problematic Token.
    void FoundIllegal(
        Command& command,
        std::string sErrorContent,
        std::string sHint,
        size_t uTokenIndex
);
};