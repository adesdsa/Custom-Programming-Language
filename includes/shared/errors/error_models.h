#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <cstdint>
#include <sstream>

constexpr int IGNORE_LINE = -1;
constexpr int IGNORE_COL = -1;

namespace ErrorTypes
{
    inline constexpr std::string_view INTERNAL_ERROR = "internal error";
    inline constexpr std::string_view SYNTAX_ERROR = "syntax error";
    inline constexpr std::string_view TYPE_ERROR = "type error";
    inline constexpr std::string_view NAME_ERROR = "name error";
    inline constexpr std::string_view LINKING_EROR = "linking error";
    inline constexpr std::string_view ARGUMENT_ERROR = "argument error";
    inline constexpr std::string_view VALUE_ERROR = "value error";
    inline constexpr std::string_view ACCESS_RIGHT_ERROR = "access rights error";
    inline constexpr std::string_view FUNC_MISSING_RETURN_ERROR = "not all function paths returns value error";
    inline constexpr std::string_view STATEMENT_OUT_OF_BLOCK_ERROR = "statement out of block error";
    inline constexpr std::string_view UNDEFINED_FUNCTION_ERROR = "function is not defined error";
}

class CompileTimeError
{
public:
    CompileTimeError(
        std::string_view type, 
        std::string content, 
        size_t line, 
        size_t column, 
        std::string hint
    );

    std::string ToString() const noexcept;

    void SetFilePath(const std::string& filePath);

private:
    std::string_view m_type;
    std::string m_content;
    size_t m_line;
    size_t m_column;
    std::string m_hint;
    std::string m_filePath;
};

class CompilationAbortException : public std::exception 
{
public:
    const char* what() const noexcept override;
};