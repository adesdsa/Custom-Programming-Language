#pragma once

#include "includes/tokenizer/token.h"

class TokenClassifier
{
public:
    // prevent creating instances
    TokenClassifier() = delete;

    // The func compares if the Token's value equals to the string.
    // @param token: The token we are comparing.
    // @param sCompare: The string we compare.
    // @return: True-> the Token's value equals the string. False-> otherwise.
    static bool IsValue(const Token& token, const std::string& sCompare);

    // The func compares if the Token's type equals to a TokenType.
    // @param token: The token we are comparing.
    // @param sCompare: The string_view which is a TokenType we compare.
    // @return: True-> the Token's type equals the given type.
    // False-> otherwise.
    static bool IsType(const Token& token, std::string_view sCompare);

    // The func checks if the given token is an Access right.
    // @param token: The token we are checking.
    // @return: True-> the Token is an Access Right. False-> otherwise.
    static bool IsAccessRights(const Token& token);

    // The func checks if the given token is a Scope.
    // @param token: The token we are checking.
    // @return: True-> the Token is a Scope. False-> otherwise.
    static bool IsScope(const Token& token);

    // The func checks if the given token is an expression start.
    // @param token: The token we are checking.
    // @return: True-> the Token is an expression start. False-> otherwise.
    static bool IsExpressionStart(const Token& token);

    // The func checks if the given token is a legal expression value.
    // @param token: The token we are checking.
    // @return: True-> the Token is a legal expression value. 
    // False-> otherwise.
    static bool IsValueToken(const Token& token);

    // The func checks if the given token is an expression operator.
    // @param token: The token we are checking.
    // @return: True-> the Token is an expression operator. False-> otherwise.
    static bool IsOperator(const Token& token);

    // The func checks if the given token is a Var Type.
    // @param token: The token we are checking.
    // @return: True-> the Token is a Var Type. False-> otherwise.
    static bool IsType(const Token& token);

    // The func checks if the given token is a control statement.
    // @param token: The token we are checking.
    // @return: True-> the Token is a control statement. False-> otherwise.
    static bool IsControlStatement(const Token& token);
};