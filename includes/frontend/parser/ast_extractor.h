#pragma once

#include <string>
#include <optional>

#include "ast.h"
#include "command.h"

#include "includes/shared/common/common_consts.h"
#include "includes/frontend/semantic_analysis/var_factory.h"

/// @brief Extracts raw values from AST nodes
/// @note Pure utility class, no validation just extraction
class ASTExtractor
{
public:
    static const std::string& ExtractVarName(ASTNode* varDecNode);
    static Type ExtractVarType(ASTNode* varDecNode);
    static Scope ExtractVarScope(ASTNode* varDecNode);
    static AccessRights ExtractVarAccessRight(ASTNode* varDecNode);

    static Type ExtractFuncReturnType(ASTNode* funcDecNode);
    static const std::vector<VarMetaData> ExtractFuncParams(
        ASTNode* funcDecNode, 
        const VarFactory& varFactory
    );

private:
    static std::optional<Type> StringToType(std::string_view type);
};