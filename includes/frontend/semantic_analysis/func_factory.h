#pragma once

#include "func_table.h"
#include "includes/frontend/common/project_symbol_table.h"
#include "includes/frontend/semantic_analysis/var_factory.h"

#define NO_PARAMS {}

class FuncFactory
{
public:
    FuncFactory(const EntityLogger& entityLogger);
    ~FuncFactory() = default;

    FuncMetaData CreateFuncMetadata(
        const std::string& funcName,
        Type funcRtype,
        const std::vector<VarMetaData>& funcParams,
        size_t declarationLine,
        size_t declarationColumn,
        VarTableNode* functionScopeRoot = nullptr
    ) const;

    FuncMetaData CreateFuncMetadata(const ExportedFunc& exportedFunc) const;

private:
    const EntityLogger& m_entityLogger;
};