#include "includes/semantic_analysis/func_factory.h"

FuncFactory::FuncFactory(const EntityLogger &entityLogger) : m_entityLogger(entityLogger) {}

FuncMetaData FuncFactory::CreateFuncMetadata(
    const std::string& funcName,
    Type funcRtype,
    const std::vector<VarMetaData>& funcParams,
    size_t declarationLine,
    size_t declarationColumn,
    std::unique_ptr<VarTableNode> funcBody
) const
{
    return FuncMetaData(
        this->m_entityLogger,
        funcName,
        funcRtype,
        funcParams,
        declarationLine,
        declarationColumn,
        std::move(funcBody)
    );
}