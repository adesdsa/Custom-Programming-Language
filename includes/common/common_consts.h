#pragma once

#include <string>

enum class Scope
{
    GLOBAL,
    VAR,
    LET
};

enum class Type
{
    VOID_TYPE,
    BOOL_TYPE,
    INT_TYPE,
    DOUBLE_TYPE, 
    UNSIGNED_INT_TYPE,
    UNSIGNED_DOUBLE_TYPE,
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
    UNKNOWN,
    VOID_TYPE
};
