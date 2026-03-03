#pragma once

#include <string>

// indexes of tokens
constexpr int FIRST_TOKEN_INDEX = 0;
constexpr int IDENTIFIER_NAME_INDEX = 0;
constexpr int IDENTIFIER_INDEX = 0;
constexpr int FIRST_PARAMETER_INDEX = 0;
constexpr int FUNCTION_NAME_TOKEN_INDEX = 0;
constexpr int VAR_NAME_TOKEN_INDEX = 0;
constexpr int CONDITION_TOKEN_INDEX = 0;
constexpr int STATEMENT_TOKEN_INDEX = 0;

enum class Scope
{
    GLOBAL,
    VAR,
    LET,
    EXPORT
};

enum class Type
{
    VOID_TYPE,
    BOOL_TYPE,
    INT_TYPE,
    DOUBLE_TYPE, 
    UNSIGNED_INT_TYPE,
    STRING_TYPE  // string literal
};

enum class AccessRights
{
    DEFAULT_ACCESS,
    CONST_ACCESS,
    READ_ONLY_ACCESS
};

enum class ExpressionType
{
    BOOLEAN,
    MATHEMATICAL,
    STRING,               // string literal expression
    UNKNOWN,
    VOID_TYPE,
    PARANTHESES_OPEN,
    PARANTHESES_CLOSE,
    COMPARRISON_OPERATOR,
    BITWISE_OPERATOR          // xor – adapts to operand types (int→int, bool→bool)
};
