#pragma once

#include <string>
#include <optional>

#include "ast.h"
#include "command.h"

#include "includes/common/common_consts.h"

/// @brief Extracts raw values from AST nodes
/// @note Pure utility class, no validation just extraction
class ASTExtractor
{
public:
    // TODO: Make command IsType, IsValue, IsScope, IsAccessRight a static methods
    static std::optional<std::string> ExtractVarName(ASTNode* varDecNode);
    static std::optional<Type> ExtractVarType(ASTNode* varDecNode);
    static Scope ExtractVarScope(ASTNode* varDecNode);
    static AccessRights ExtractVarAccessRight(ASTNode* varDecNode);

private:
    static std::optional<Type> StringToType(std::string_view type);
};