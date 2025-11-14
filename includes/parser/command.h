#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "tokenizer/token.h"

namespace CommandTypes
{
    inline constexpr std::string_view UNIDENTIFIED              = "UNIDENTIFIED";
    inline constexpr std::string_view IDENTIFIER                = "IDENTIFIER";
    inline constexpr std::string_view UNKNOWN                   = "UNKNOWN";
    inline constexpr std::string_view FUNC_START                = "FUNC_START";
    inline constexpr std::string_view FUNC_CALL                 = "FUNC_CALL";
    inline constexpr std::string_view FILLED_FUNC_DECLARATION   = "FILLED_FUNC_DECLARATION";
    inline constexpr std::string_view ARROW_FUNC_DECLARATION    = "ARROW_FUNC_DECLARATION";
    inline constexpr std::string_view FUNC_DECLARATION          = "FUNC_DECLARATION";
}

class Command 
{
public:
    Command();
    // The func tries adding a new token to the command to make 
    bool Append(Token currToken);

private:
    std::vector<Token> m_tokens;
    std::vector<Command> m_parameters;
    size_t m_uParameterIndex;
    std::string m_sType;

    bool CheckParamsCalls();
    bool CheckParamsDeclarations();
    bool IsType();

}