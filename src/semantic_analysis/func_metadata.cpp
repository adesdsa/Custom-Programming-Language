#include "includes/semantic_analysis/func_metadata.h"

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
}

int FuncMetaData::ComputeFrameSize()
{
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
        minOffset = std::min(minOffset, FindDeepestOffset(child.get()));
    }

    return minOffset;
}
