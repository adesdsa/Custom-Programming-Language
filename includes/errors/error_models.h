#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <cstdint>
#include <sstream>

namespace ErrorTypes
{
    inline constexpr std::string_view SYNTAX_ERROR = "syntax error";
    inline constexpr std::string_view TYPE_ERROR = "type error";
    inline constexpr std::string_view NAME_ERROR = "name error";
    inline constexpr std::string_view LINKING_EROR = "linking error";
}

// TODO: Add field of path to file in which the error occured
class CompileTimeError : public std::exception
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

class CompilationAbortException : public std::exception 
{
public:
    const char* what() const noexcept override;
};