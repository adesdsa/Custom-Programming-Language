#pragma once

#include <string_view>
#include <unordered_map>

namespace CommandTypes
{
    /// 
    ///             The next CommandTypes are the finished CommandTypes.
    ///
    inline constexpr std::string_view IDENTIFIER                    = "IDENTIFIER";
    inline constexpr std::string_view UNKNOWN                       = "UNKNOWN";
    inline constexpr std::string_view END_OF_COMMAND                = "END_OF_COMMAND";
    inline constexpr std::string_view BLOCK_START                   = "BLOCK_START";
    inline constexpr std::string_view BLOCK_END                     = "BLOCK_END";
    inline constexpr std::string_view ELSE                          = "ELSE";
    inline constexpr std::string_view RETURN                        = "RETURN";
    inline constexpr std::string_view ASSIGNMENT                    = "ASSIGNMENT";
    inline constexpr std::string_view VOID_COMMAND                  = "VOID_COMMAND";
    inline constexpr std::string_view POINTER_DECLARATION           = "POINTER_DECLARATION";
    inline constexpr std::string_view CONTROL_STATEMENT             = "CONTROL_STATEMENT";
    inline constexpr std::string_view VAR_DECLARATION               = "VAR_DECLARATION";
    inline constexpr std::string_view EOF_TOKEN                     = "EOF_TOKEN";
    inline constexpr std::string_view EXPRESSION                    = "EXPRESSION";
    inline constexpr std::string_view PASS                          = "PASS";
    inline constexpr std::string_view ARRAY_VALUE                   = "ARRAY_VALUE";
    inline constexpr std::string_view POINTER_USE                   = "POINTER_USE";


    /// 
    ///             The next CommandTypes are the finished CommandTypes that need to be reformed in CommandLine.
    ///
    inline constexpr std::string_view ELIF                          = "ELIF";
    inline constexpr std::string_view IF                            = "IF";
    inline constexpr std::string_view FOR                           = "FOR";
    inline constexpr std::string_view WHILE                         = "WHILE";
    inline constexpr std::string_view SIZEOF                        = "SIZEOF";
    inline constexpr std::string_view FINISHED_FUNC_DECLARATION     = "FINISHED_FUNC_DECLARATION";
    inline constexpr std::string_view FINISHED_FUNC_CALL            = "FINISHED_FUNC_CALL";
    inline constexpr std::string_view ARRAY_ACCESS                  = "ARRAY_ACCESS";
    inline constexpr std::string_view ARRAY_DECLARATION             = "ARRAY_DECLARATION";
    inline constexpr std::string_view VAR_TYPE                      = "VAR_TYPE";


    ///
    ///             The next CommandTypes must be reformed and aren't finished yet.
    ///
    inline constexpr std::string_view EXPRESSION_VALUE              = "EXPRESSION_VALUE";
    inline constexpr std::string_view EXPRESSION_OPERATOR           = "EXPRESSION_OPERATOR";


    /// 
    ///             The next CommandTypes are the non finished CommandTypes
    ///
    inline constexpr std::string_view UNIDENTIFIED                  = "UNIDENTIFIED";
    inline constexpr std::string_view FUNC_START                    = "FUNC_START";
    inline constexpr std::string_view FILLED_FUNC_DECLARATION       = "FILLED_FUNC_DECLARATION";
    inline constexpr std::string_view ARROW_FUNC_DECLARATION        = "ARROW_FUNC_DECLARATION";
    inline constexpr std::string_view ACCESS_RIGHTS                 = "ACCESS_RIGHTS";
    inline constexpr std::string_view SCOPE                         = "SCOPE";
    inline constexpr std::string_view VAR_DECLARATION_START         = "VAR_DECLARATION_START";
    inline constexpr std::string_view ELIF_CALL                     = "ELIF_CALL";
    inline constexpr std::string_view IF_CALL                       = "IF_CALL";
    inline constexpr std::string_view ELIF_START                    = "ELIF_START";
    inline constexpr std::string_view IF_START                      = "IF_START";
    inline constexpr std::string_view FOR_CALL                      = "FOR_CALL";
    inline constexpr std::string_view WHILE_CALL                    = "WHILE_CALL";
    inline constexpr std::string_view FOR_START                     = "FOR_START";
    inline constexpr std::string_view WHILE_START                   = "WHILE_START";
    inline constexpr std::string_view SIZEOF_CALL                   = "SIZEOF_CALL";
    inline constexpr std::string_view SIZEOF_START                  = "SIZEOF_START";
    inline constexpr std::string_view ARRAY_ACCESS_START            = "ARRAY_ACCESS_START";
    inline constexpr std::string_view NOT_FUNC_IDENTIFIER           = "NOT_FUNC_IDENTIFIER";
    inline constexpr std::string_view ARRAY_DECLARATION_START       = "ARRAY_DECLARATION_START";
    inline constexpr std::string_view ARRAY_VALUE_START             = "ARRAY_VALUE_START";
}
