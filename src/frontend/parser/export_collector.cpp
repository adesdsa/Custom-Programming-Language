#include "includes/frontend/parser/export_collector.h"

FileExportTable CollectExports(const AST& ast, const std::string& filePath)
{
    FileExportTable table;
    table.filePath = filePath;
    table.fileName = std::filesystem::path(filePath).stem().string();

    ASTNode* cursor = ast.GetRoot();
    bool nextFuncIsExported = false;

    while (cursor != nullptr)
    {
        std::string_view type = cursor->command.GetType();

        if (type == CommandTypes::VAR_DECLARATION)
        {
            Scope scope = ASTExtractor::ExtractVarScope(cursor);
            if (scope == Scope::EXPORT)
            {
                ExportedVar var;
                var.name = ASTExtractor::ExtractVarName(cursor);
                var.type = ASTExtractor::ExtractVarType(cursor);
                table.exportedVars.push_back(var);
            }
        }

        else if (type == CommandTypes::FINISHED_FUNC_DEFINITION ||
            type == CommandTypes::FINISHED_FUNC_DECLARATION)
        {
            ExportedFunc func;
            func.name = cursor->command.GetTokens()[FUNCTION_NAME_TOKEN_INDEX].GetValue();
            func.returnType = ASTExtractor::ExtractFuncReturnType(cursor);

            // walk children to get parameter signatures
            for (ASTNode* param = cursor->nodeFirstChild;
                 param != nullptr;
                 param = param->nodeNextSibling)
            {
                if (param->command.GetType() == CommandTypes::VAR_DECLARATION)
                {
                    ExportedVar p;
                    p.name = ASTExtractor::ExtractVarName(param);
                    p.type = ASTExtractor::ExtractVarType(param);
                    func.params.push_back(p);
                }
            }
            table.functions.push_back(func);
        }

        // skip function bodies
        if (type == CommandTypes::BLOCK_START)
        {
            int depth = 1;
            while (cursor->nodeNextSibling && depth > 0)
            {
                cursor = cursor->nodeNextSibling;
                if (cursor->command.GetType() == CommandTypes::BLOCK_START) depth++;
                if (cursor->command.GetType() == CommandTypes::BLOCK_END) depth--;
            }
        }

        cursor = cursor->nodeNextSibling;
    }

    return table;
}
