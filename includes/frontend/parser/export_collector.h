#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "includes/frontend/common/project_symbol_table.h"
#include "includes/frontend/parser/ast.h"
#include "includes/frontend/parser/ast_extractor.h"

/// @brief Walks the AST top-level returns all importable symbols
FileExportTable CollectExports(const AST& ast, const std::string& filePath);

/// @brief Walks the AST top-level and returns module stems of all imported files
std::vector<std::string> CollectImports(const AST& ast);
