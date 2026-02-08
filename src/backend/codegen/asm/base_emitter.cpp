#include "includes/backend/codegen/asm/base_emitter.h"

BaseEmitter::BaseEmitter(const FuncTable& funcTable) :
    m_funcTable(funcTable),
    m_currentFunc(nullptr)
{}

std::string BaseEmitter::Emit(const IRModule& module)
{
    std::ostringstream out;

    out << EmitHeader();
    EmitDataSection(module, out);
    EmitBssSection(module, out);
    EmitTextSection(module, out);

    return out.str();
}

void BaseEmitter::EmitDataSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::DATA);

    for (const auto& globalVar : module.GetVariables())
    {
        if (globalVar.get()->GetInitializer())
        {
            int varSize = GetTypeSize(globalVar->GetType());
            out << Indent() << globalVar->GetName() << ": ";
            out << GetDataDirective(varSize) << " ";
            out << globalVar->GetInitializerString() << "\n";
        }
    }
}
    
void BaseEmitter::EmitBssSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::BSS);
    
    for (const auto& global : module.GetVariables())
    {
        if (!global.get()->GetInitializer())
        {
            int size = GetTypeSize(global->GetType());
            out << Indent() << global->GetName() << ": ";
            /// @note we only save scalar (no arrays) so only 1 count is needed
            out << GetReserveDirective(size) << " 1\n";
        }
    }
}

void BaseEmitter::EmitTextSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::TEXT);
    
    EmitStart(out);
    out << "\n";
    
    for (const auto& func : module.GetFunctions())
    {
        EmitFunction(func.get(), out);
        out << "\n";
    }

    out << ".func_epilogue:\n";
    EmitEpilogue(out);
}

void BaseEmitter:: EmitFunction(const IRFunction* func, std::ostream& out)
{
    FuncMetaData* funcMetadata = this->m_funcTable.FindFunctionByName(func->GetName());
    if (!funcMetadata) return;
    this->m_currentFunc = func;

    // function label
    out << func->GetName() << ":\n";

    int frameSize = funcMetadata->ComputeFrameSize();
    EmitPrologue(func, frameSize, out);

    for (const auto& block : func->GetBasicBlocks())
    {
        EmitBasicBlock(block.get(), out);
    }
    
    this->m_currentFunc = nullptr;
}

void BaseEmitter::EmitBasicBlock(const BasicBlock* block, std::ostream& out)
{
    out << "." << block->GetLabel() << ":\n";

    for (const auto& instr : block->GetInstructions())
    {
        EmitInstruction(instr.get(), out);
    }
}

int BaseEmitter::GetTypeSize(std::shared_ptr<const IRType> type) const
{
    switch (type->GetKind())
    {
    case IRTypeKind::Float32:
        return 4;

    case IRTypeKind::Float64:
        return 8;

    case IRTypeKind::Int1:
        return 1;

    case IRTypeKind::Int32:
        return 4;

    case IRTypeKind::Int64:
        return 8;

    case IRTypeKind::Int8:
        return 1;
    
    default: // Incase of error or void (shouldnt get here)
        return 0;
    }
}

std::string BaseEmitter::Indent() const
{
    return "    "; // 4 spaces (or "\t" if you prefer tabs)
}
