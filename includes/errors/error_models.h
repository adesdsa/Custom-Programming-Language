#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <sstream>

namespace ErrorTypes
{
    inline constexpr std::string_view SYNTAX_ERROR = "Syntax Error";
    inline constexpr std::string_view TYPE_ERROR = "Type Error";
    inline constexpr std::string_view NAME_ERROR = "Name Error";
    inline constexpr std::string_view LINKING_EROR = "Linking Error";
}

// TODO: Add field of path to file in which the error occured
class CompileTimeError
{
public:
    CompileTimeError(
        std::string_view type, 
        std::string content, 
        size_t line, 
        uint16_t column, 
        std::string hint
    );

    std::string ToString() const noexcept;

private:
    std::string_view m_type;
    std::string m_content;
    size_t m_line;
    uint16_t m_column;
    std::string m_hint;
};