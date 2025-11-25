#pragma once

#include <string>
#include <string_view>
#include <cstdint>

struct CompileTimeError
{
    std::string_view type;
    std::string content;
    size_t line;
    uint16_t column;
    std::string hint;

    CompileTimeError(
        std::string_view type, 
        std::string content, 
        size_t line, 
        uint16_t column, 
        std::string hint
    ) :
        type(type), content(content), line(line), column(column), hint(hint) {}
};

namespace ErrorTypes
{
    inline constexpr std::string_view SYNTAX_ERROR = "Syntax Error";
    inline constexpr std::string_view TYPE_ERROR = "Type Error";
    inline constexpr std::string_view NAME_ERROR = "Name Error";
    inline constexpr std::string_view LINKING_EROR = "Linking Error";
}