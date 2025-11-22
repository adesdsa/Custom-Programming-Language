#pragma once

#include "parser/command.h"


class CommandLine{
public:
    CommandLine(const EntityLogger& entityLogger);
    bool IsFinished();
    bool AddToken(Token currToken, Token nextToken);

private:
    const EntityLogger& m_entityLogger;
    bool m_bFinished;
    std::vector<Command> m_line;
    size_t m_uLineIndex;

    void CheckLine();

};