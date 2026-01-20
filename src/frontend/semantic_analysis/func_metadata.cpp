#include "includes/frontend/semantic_analysis/func_metadata.h"

FuncMetaData::FuncMetaData(
    const EntityLogger& entityLogger,
    const std::string& funcName,
    Type funcRtype,
    const std::vector<VarMetaData>& funcParams,
    size_t declarationLine,
    size_t declarationColumn,
    std::unique_ptr<VarTableNode> functionScopeRoot
) :
    m_entityLogger(entityLogger),
    m_name(funcName),
    m_rtype(funcRtype),
    m_params(funcParams),  // case of no params funcParams will be {}
    m_declarationLine(declarationLine),
    m_declarationColumn(declarationColumn),
    m_functionScopeRoot(std::move(functionScopeRoot)) 
{
    // if there is body to the function, it definition
    if (this->m_functionScopeRoot)
    {
        this->m_isDefined = true;
    }

    this->m_isDeclared = true;
}

const std::string &FuncMetaData::GetName() const noexcept
{
    return this->m_name;
}

const Type& FuncMetaData::GetRType() const noexcept
{
    return this->m_rtype;
}

const std::vector<VarMetaData>& FuncMetaData::GetFuncParams() const noexcept
{
    return m_params;
}

bool FuncMetaData::IsFuncDefined() const noexcept
{
    return this->m_isDefined;
}

bool FuncMetaData::IsFuncDeclared() const noexcept
{
    return this->m_isDeclared;
}

size_t FuncMetaData::GetDeclarationLine() const noexcept
{
    return this->m_declarationLine;
}

size_t FuncMetaData::GetDeclarationColumn() const noexcept
{
    return this->m_declarationColumn;
}

void FuncMetaData::SetDefined() noexcept
{
    this->m_isDefined = true;
}

bool FuncMetaData::HasSameSignature(const FuncMetaData& other) const
{
    if (this->m_name != other.m_name) return false;
    if (this->m_rtype != other.m_rtype) return false;
    if (this->m_params.size() != other.m_params.size()) return false;

    // make sure both signatures params are the same
    for (size_t param = 0; param < this->m_params.size(); param++)
    {
        if (this->m_params[param].type != other.m_params[param].type)
        {
            return false;
        }
    }

    return true;
}

int FuncMetaData::ComputeFrameSize()
{
    this->m_entityLogger.Info("Computing function " + this->m_name + "'s frame size");

    // no function body, no need for allocating space
    if (!this->m_functionScopeRoot)
    {
        this->m_stackFrameSize = 0;
        return 0;
    }

    // furthest offset from stack frame will be 
    // number of bytes to alloceate for the stack later on
    int deepestOffset = FindFurthestOffset(this->m_functionScopeRoot.get());
    int frameSize = -deepestOffset;  // offsets are negative

    // adjust for ABI alignment x86-64
    if (frameSize % ALIGNMENT != 0)
    {
        frameSize += ALIGNMENT - (frameSize % ALIGNMENT);
    }

    this->m_stackFrameSize = frameSize;
    return frameSize;
}

int FuncMetaData::FindFurthestOffset(const VarTableNode* node)
{
    if (!node) return 0;

    // track based on min becasue all offsets are negative
    int minOffset = 0;

    for (const auto& [_, var] : node->GetVars())
    {
        minOffset = std::min(minOffset, var.offsetInsideFrame);
    }

    for (const auto& child : node->GetChildren())
    {
        minOffset = std::min(minOffset, FindFurthestOffset(child.get()));
    }

    return minOffset;
}
