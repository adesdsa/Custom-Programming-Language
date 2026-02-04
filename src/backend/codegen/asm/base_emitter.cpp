#include "includes/backend/codegen/asm/base_emitter.h"

std::string BaseEmitter::Emit(const IRModule& module) override final;
{
    std::ostringstream out;
    out << EmitHeader();
    out << EmitDataSection(module);
    out << EmitBssSection(module);
    out << EmitTextSection(module);
    return out.str();
}

void BaseEmitter::EmitDataSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::DATA);

    for (const auto& globalVar : module.GetVariables())
    {
        if (globalVar->HasInitializer())
        {
            int varSize = GetTypeSize(globalVar->GetType().get());
            out << Indent() << globalVar->GetName() << ": ";
            out << GetDataDirective(size) << " ";
            out << globalVar->GetInitializerString() << "\n";
        }
    }
}
    
void BaseEmitter::EmitBssSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::BSS);
    
    for (const auto& global : module.GetVariables())
    {
        if (!global->HasInitializer())
        {
            int size = GetTypeSize(global->GetType().get());
            out << Indent() << global->GetName() << ": ";
            // TODO: The 1 here means the quantity to reserve, check when do we need more than 1
            out << GetReserveDirective(size) << " 1\n";
        }
    }
}

void BaseEmitter::EmitTextSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::TEXT);
    
    // export main if it exists
    if (module.IsFuncExistByName("main"))
    {
        out << Indent() << GetGlobalDirective("main");
    }
    out << "\n";
    
    for (const auto& func : module.GetFunctions())
    {
        EmitFunction(func.get(), out);
        out << "\n";
    }
}

void BaseEmitter::EmitFunction(const IRFunction* func, std::ostream& out)
{
    this->m_currentFunc = func;

    // function label
    out << func->GetName() << ":\n";

    int frameSize = funcMetadata.ComputeStackSize();
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
