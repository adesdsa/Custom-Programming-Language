#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "tokenizer/token.h"

namespace CommandTypes
{
    inline constexpr std::string_view UNIDENTIFIED                  = "UNIDENTIFIED";
    inline constexpr std::string_view IDENTIFIER                    = "IDENTIFIER";
    inline constexpr std::string_view UNKNOWN                       = "UNKNOWN";
    inline constexpr std::string_view FUNC_START                    = "FUNC_START";
    inline constexpr std::string_view FUNC_CALL                     = "FUNC_CALL";
    inline constexpr std::string_view FILLED_FUNC_DECLARATION       = "FILLED_FUNC_DECLARATION";
    inline constexpr std::string_view ARROW_FUNC_DECLARATION        = "ARROW_FUNC_DECLARATION";
    inline constexpr std::string_view FUNC_DECLARATION              = "FUNC_DECLARATION";
    inline constexpr std::string_view ACCESS_RIGHTS                 = "ACCESS_RIGHTS";
    inline constexpr std::string_view SCOPE                         = "SCOPE";
    inline constexpr std::string_view VAR_DECLARATION_START         = "VAR_DECLARATION_START";
    inline constexpr std::string_view VAR_DECLARATION               = "VAR_DECLARATION";
    inline constexpr std::string_view END_OF_COMMAND                = "END_OF_COMMAND";
    inline constexpr std::string_view VAR_DECLARATION_ASSIGNMENT    = "VAR_DECLARATION_ASSIGNMENT";
    inline constexpr std::string_view VAR_ASSIGNMENT                = "VAR_ASSIGNMENT";
    inline constexpr std::string_view VAR_ASSIGNMENT_START          = "VAR_ASSIGNMENT_START";
    inline constexpr std::string_view BLOCK_START                   = "BLOCK_START";
    inline constexpr std::string_view BLOCK_END                     = "BLOCK_END";
    inline constexpr std::string_view ELSE                          = "ELSE";
    inline constexpr std::string_view ELIF_CALL                     = "ELIF_CALL";
    inline constexpr std::string_view IF_CALL                       = "IF_CALL";
    inline constexpr std::string_view ELIF_START                    = "ELIF_START";
    inline constexpr std::string_view IF_START                      = "IF_START";
    inline constexpr std::string_view ELIF                          = "ELIF";
    inline constexpr std::string_view IF                            = "IF";
    inline constexpr std::string_view FOR_CALL                      = "FOR_CALL";
    inline constexpr std::string_view WHILE_CALL                    = "WHILE_CALL";
    inline constexpr std::string_view FOR_START                     = "FOR_START";
    inline constexpr std::string_view WHILE_START                   = "WHILE_START";
    inline constexpr std::string_view FOR                           = "FOR";
    inline constexpr std::string_view WHILE                         = "WHILE";
    inline constexpr std::string_view RETURN_START                  = "RETURN_START";
    inline constexpr std::string_view EXPRESSION_VALUE              = "EXPRESSION_VALUE";
    inline constexpr std::string_view EXPRESSION_OPERATOR           = "EXPRESSION_OPERATOR";
    inline constexpr std::string_view PARANTHESES                   = "PARANTHESES";
}

class Command 
{
public:
    Command(); // todo: handle this shit later
    // The func tries adding a new token to the command to make 
    bool Append(Token currToken, Token nextToken);

private:
    std::vector<Token> m_tokens;
    std::vector<Command> m_parameters;
    size_t m_uParameterIndex;
    std::string m_sType;
    const EntityLogger& m_entityLogger;
    bool m_bBuildingParam;
    unsigned int m_uParanthesesCounter;

    bool CheckParamsValues();
    bool CheckParamsDeclarations();
    bool IsType();
    bool IsAccessRights();
    bool IsScope();
    bool FoundUnknown(); //m_sType = CommandTypes::UNKNOWN; return true;
    bool IsBoolean(); // includes bool keywords and bool operators and bool values
    bool IsControlStatement(); // includes continue pass and break
    bool IsLoopStatement(); // includes for and while
    bool IsLogicStatement(); // includes if elif and else
    bool IsExpressionStart();
    // only return sizeof is xor are not handled

    // true->legal,false->illegal
    bool HandleParams(Token currToken, Token nextToken);
    bool HandleParantheses(Token currToken, Token nextToken);

    // todo: create a [] operator for a foreach loop for easier param access

}