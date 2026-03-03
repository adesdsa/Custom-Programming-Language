#pragma once

#include <filesystem>

#include "includes/frontend/common/project_symbol_table.h"
#include "includes/frontend/parser/ast.h"
#include "includes/frontend/parser/ast_extractor.h"

/// @brief Walks the AST top-level returns all importable symbols
FileExportTable CollectExports(const AST& ast, const std::string& filePath);
