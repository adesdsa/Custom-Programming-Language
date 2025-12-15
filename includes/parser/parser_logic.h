#pragma once

#include "ast.h"
#include "full_command.h"

#include "includes/tokenizer/lexer.h"
#include "includes/errors/error_manager.h"

class ParserLogic
{
public:
    ParserLogic(const std::filesystem::path& filePath);
    ~ParserLogic() = default;

    /*****************************************************************************
     * @brief Runs Parser Pipeline:                                              |
     *                                                                           |
     * This method performs the complete parsing process:                        |
     * 1. Building FullCommand objects by fetching tokens on demand.             |
     *  @note: A full command may represent a function declaration,              |
     *          variable declaration, condition statement, etc....               |
     *                                                                           |
     * 2. Each full command goes through SyntaxChecker.                          |
     *                                                                           |
     * 3. If the command passes syntax validation, insert into the AST           |
     *      as a new subtree.                                                    |
     *                                                                           |
     * 4. If command is invalid, syntax errors are recorded via error manager    |                                   |
     *                                                                           |
     * 5. Repeats proccess from step 1 until the source input is fully consumed  |
     *                                                                           |
     * 6. If any errors occured, handle them and abort compilation               |
     *                                                                           |
     * 7. Otherwise, outputs the built AST                                       |
     *                                                                           |
     * @see Command                                                              |
     * @see FullCommand                                                          |
     * @see AST                                                                  |
     ****************************************************************************/
    const AST& MainFlow();

    // Sets m_currentToken to be the m_nextToken,
    // and fetch new m_nextToken from lexer.
    void Advance();

    // A command line is considered valid if it passed the SyntaxChecker layer.
    // If `fullCommand` is valid -> construct its subtree in the AST.
    // Otherwise -> record the syntax errors in `m_errorManager`.
    // @param fullCommand: The fully command line to analyze and process.
    void HandleFullCommand(const FullCommand& fullCommand);

private:
    // infrastructures
    EntityLogger m_entityLogger;
    ErrorManager& m_errorManager;

    // inner components
    AST m_ast;
    Lexer m_lexer;

    // data
    Token m_currentToken;
    Token m_nextToken;


    // Gets new relevant token from lexer
    Token FetchToken();
};
