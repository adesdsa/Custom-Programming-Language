#pragma once

#include "command.h"
#include "logger/entity_logger.h"


class CommandLine{
public:
    CommandLine(const EntityLogger& entityLogger);
    CommandLine(const CommandLine& other);
    
    CommandLine& operator=(const CommandLine& other);

    // The func is used to add token to the line.
    // When the line is finished it will check it's legallity
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


private:
    const EntityLogger& m_entityLogger;
    bool m_bFinished;
    bool m_bLegal;
    std::vector<Command> m_line;
    size_t m_uLineIndex;
    static unsigned int m_uBlockStartCnt;
    static unsigned int m_uBlockCloseCnt;

    // The func is used to check if the line is legal.
    // It will save the result in the func's params.
    void CheckLine();

    void ReformCommand(Command& checkedCommand);

    bool CheckConstSize(Command& checkedCommand, int iSize);

    bool CheckParantheses(Command& checkedCommand);

    bool CheckParamExpectedExpression(Command& checkedCommand, int iParamIndex);

    bool CheckParamExpectedTypes(Command& checkedCommand, int iParamIndex, std::vector<std::string_view> types);
    
    bool CheckLastFuncParamComa(Command& checkedCommand, int iBackSteps);

    void HandleSingleParamKeyword(Command& checkedCommand);

    void HandleFuncLikeExpressions(Command& checkedCommand, int iStartingIndex); // anything that is like that identifier+(any kind of paranthese)+(expression+,)+...(last expression shouldn't have ,)+(same kind of paranthese)

    inline bool CheckLastExpressionOperator(Command& checkedCommand);

    inline bool CheckType(const Token& token, std::string_view sCompare) const;

    inline bool CheckValue(const Token& token, std::string sCompare) const;

    void FoundIllegal(Command& command);

};