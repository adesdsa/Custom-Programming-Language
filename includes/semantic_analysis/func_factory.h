#pragma once

#include "func_table.h"

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
        std::unique_ptr<VarTableNode> functionScopeRoot = nullptr
    ) const;

private:
    const EntityLogger& m_entityLogger;
};