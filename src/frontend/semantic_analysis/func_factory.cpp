#include "includes/frontend/semantic_analysis/func_factory.h"

FuncFactory::FuncFactory(const EntityLogger &entityLogger) : m_entityLogger(entityLogger) {}

FuncMetaData FuncFactory::CreateFuncMetadata(
    const std::string& funcName,
    Type funcRtype,
    const std::vector<VarMetaData>& funcParams,
    size_t declarationLine,
    size_t declarationColumn,
    VarTableNode* funcBody
) const
{
    return FuncMetaData(
        this->m_entityLogger,
        funcName,
        funcRtype,
        funcParams,
        declarationLine,
        declarationColumn,
        funcBody
    );
}

FuncMetaData FuncFactory::CreateFuncMetadata(const ExportedFunc& exportedFunc) const
{
    std::vector<VarMetaData> funcParams;
    VarFactory varFactory(this->m_entityLogger);
    for (auto funcParam : exportedFunc.params)
    {
        funcParams.push_back(varFactory.CreateVarMetadata(funcParam));
    }

    return FuncMetaData(
        this->m_entityLogger,
        exportedFunc.name,
        exportedFunc.returnType,
        funcParams,
        -1,
        -1,
        nullptr,
        true
    );
}