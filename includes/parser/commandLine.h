#pragma once

#include "parser/command.h"


class CommandLine{
public:
    CommandLine(const EntityLogger& entityLogger);

    // The func is used to add token to the line.
    // When the line is finished it will check it's legallity
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token that will be added.
    // @return: True-> Finished the line. False-> Otherwise.
    bool AddToken(Token& currToken, Token& nextToken);

    // Getters

    // Is The Line Finished.
    bool IsFinished() const;
    // Is The Line LEgal.
    bool IsLegal() const;
    // Get The Line Itself.
    const std::vector<Command>& GetLine() const;


private:
    const EntityLogger& m_entityLogger;
    bool m_bFinished;
    bool m_bLegal;
    std::vector<Command> m_line;
    size_t m_uLineIndex;

    // The func is used to check if the line is legal.
    // It will save the result in the func's params.
    void CheckLine();

    void ReformCommand();

};