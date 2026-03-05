#include "includes/backend/codegen/asm/base_emitter.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_string.h"

BaseEmitter::BaseEmitter(const FuncTable& funcTable) :
    m_funcTable(funcTable),
    m_currentFunc(nullptr)
{}

std::string BaseEmitter::Emit(const IRModule& module)
{
    std::ostringstream out;

    out << EmitHeader(module);
    EmitExternAndGlobalDirectives(module, out);
    EmitDataSection(module, out);
    EmitBssSection(module, out);
    EmitTextSection(module, out);

    return out.str();
}

std::string BaseEmitter::MangledName(const std::string& name) const
{
    return name; // default: no mangling
}

void BaseEmitter::EmitExternAndGlobalDirectives(const IRModule& module, std::ostream& out)
{
    for (const auto& symbol : module.GetExports())
    {
        out << "global " << MangledName(symbol) << "\n";
    }

    for (const auto& symbol : module.GetImports())
    {
        out << "extern " << MangledName(symbol) << "\n";
    }

    if (!module.GetExports().empty() || !module.GetImports().empty())
    {
        out << "\n";
    }
}

void BaseEmitter::EmitDataSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::DATA);
    out << "\n";

    for (const auto& globalVar : module.GetVariables())
    {
        if (globalVar->IsExtern()) continue;

        if (globalVar->GetInitializer())
        {
            int varSize = GetTypeSize(globalVar->GetType());
            out << Indent() << MangledName(globalVar->GetName()) << ": ";
            out << GetDataDirective(varSize) << " ";
            out << globalVar->GetInitializerString() << "\n";
        }
    }

    for (const auto& constant : module.GetConstants())
    {
        if (auto* stringLiteral = dynamic_cast<const IRConstantString*>(constant.get()))
        {
            out << Indent() << stringLiteral->GetLabel() << " db ";
            const std::string& val = stringLiteral->GetValue();
            for (size_t i = 0; i < val.size(); ++i)
            {
                if (i > 0) out << ", ";
                unsigned char ch = static_cast<unsigned char>(val[i]);
                if (ch >= 32 && ch < 127 && ch != '"' && ch != '\\')
                {
                    // printable ASCII
                    out << "'" << val[i] << "'";
                }
                else
                {
                    // emit as numeric byte (handles \n, \t, etc.)
                    out << static_cast<int>(ch);
                }
            }
            out << ", 0\n"; // null terminator
        }
    }

    out << "\n";
    out << "\n";
}
    
void BaseEmitter::EmitBssSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::BSS);
    out << "\n";
    
    for (const auto& global : module.GetVariables())
    {
        if (global->IsExtern()) continue;

        if (!global->GetInitializer())
        {
            int size = GetTypeSize(global->GetType());
            out << Indent() << MangledName(global->GetName()) << ": ";
            /// @note we only save scalar (no arrays) so only 1 count is needed
            out << GetReserveDirective(size) << " 1\n";
        }
    }
    out << "\n";
    out << "\n";
}

void BaseEmitter::EmitTextSection(const IRModule& module, std::ostream& out)
{
    out << GetSectionDirective(SectionType::TEXT);
    out << "\n";
    
    EmitStart(module, out);
    out << "\n";
    
    for (const auto& func : module.GetFunctions())
    {
        EmitFunction(func.get(), out);
        out << "\n";
    }
    out << "\n";
    out << "\n";
}

void BaseEmitter:: EmitFunction(const IRFunction* func, std::ostream& out)
{
    FuncMetaData* funcMetadata = this->m_funcTable.FindFunctionByName(func->GetName());
    if (!funcMetadata) return;

    // skip built-in / external functions (no basic blocks = no user-defined body)
    if (func->GetNumBasicBlocks() == 0) return;
    this->m_currentFunc = func;

    // function label (mangled for the target platform)
    out << MangledName(func->GetName()) << ":\n";

    int frameSize = funcMetadata->ComputeFrameSize();
    EmitPrologue(func, frameSize, out);

    for (const auto& block : func->GetBasicBlocks())
    {
        EmitBasicBlock(block.get(), out);
    }
    
    out << MangledName(this->m_currentFunc->GetName()) << ".func_epilogue:\n";
    EmitEpilogue(out);
    this->m_currentFunc = nullptr;
    out << "\n";
    out << "\n";
}

void BaseEmitter::EmitBasicBlock(const BasicBlock* block, std::ostream& out)
{
    out << MangledName(this->m_currentFunc->GetName()) << "." << block->GetLabel() << ":\n";

    for (const auto& instr : block->GetInstructions())
    {
        EmitInstruction(instr.get(), out);
    }
    out << "\n";
    out << "\n";
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
