#include "includes/frontend/parser/parser_logic.h"

ParserLogic::ParserLogic(const std::filesystem::path& filePath) :
    m_entityLogger("PARSER"),
    m_errorManager(ErrorManager::GetInstance()),
    m_filePath(filePath),
    m_ast(m_entityLogger),
    m_lexer(filePath),
    m_currentToken(this->FetchToken()),
    m_nextToken(this->FetchToken()) 
{
    this->m_entityLogger.Info("Finished initializing parser component successfully");
}

const AST& ParserLogic::MainFlow()
{
    ErrorManager::SetCurrentReporter("PARSER");
    ErrorManager::SetCurrentFilePath(this->m_filePath.string());
    this->m_entityLogger.Info("Start parsing file");

    bool importStillLegal = true;
    while (this->m_currentToken.GetType() != TokenTypes::EOF_TOKEN)
    {
        FullCommand fullCommand(this->m_entityLogger, importStillLegal);

        // building full-command
        while (!fullCommand.IsFinished())
        {
            fullCommand.AddToken(this->m_currentToken, this->m_nextToken);
            this->Advance();
        }

        this->HandleFullCommand(fullCommand);

        // After a non-import top-level command, imports are no longer legal
        if (fullCommand.IsLegal())
        {
            for (const auto& cmd : fullCommand.GetLine())
            {
                if (cmd.GetType() != CommandTypes::IMPORT_STATEMENT &&
                    cmd.GetType() != CommandTypes::UNKNOWN)
                {
                    importStillLegal = false;
                    break;
                }
            }
        }
    }

    if (this->m_errorManager.HasErrorsOccured())
    {
        this->m_errorManager.HandleErrors();
    }

    // debug purpose only - remove comment when needed
    // this->m_ast.PrintAST();

    this->m_entityLogger.Info("Finished parsing file");
    return this->m_ast;
}

Token ParserLogic::FetchToken()
{
    auto token = this->m_lexer.Tokenize();

    if (!token)
    {
        this->m_entityLogger.Error("Failed to fetch new token from lexer");
        throw std::runtime_error("Lexer returned no token");
    }
    return std::move(*token);
}

void ParserLogic::Advance()
{
    Token nextToken = this->FetchToken();
    this->m_currentToken = std::move(this->m_nextToken);
    this->m_nextToken = std::move(nextToken);
} 

void ParserLogic::HandleFullCommand(const FullCommand& fullCommand)
{
    if (fullCommand.IsLegal())
    {
        this->m_ast.StoreFullCommand(fullCommand);
    }
    else
    {
        for(const CompileTimeError& compileTimeError : fullCommand.GetErrors())
        {
            this->m_errorManager.StoreError(compileTimeError);
        }
    }
}
