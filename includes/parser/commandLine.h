#pragma once

#include "parser/command.h"


class CommandLine{
public:
    CommandLine(const EntityLogger& entityLogger);
    bool AddToken(Token currToken, Token nextToken);

    // Getters and Setters
    bool IsFinished() const;
    bool IsLegal() const;
    const std::vector<Command>& GetLine() const;


private:
    const EntityLogger& m_entityLogger;
    bool m_bFinished;
    bool m_bLegal;
    std::vector<Command> m_line;
    size_t m_uLineIndex;

    void CheckLine();

};