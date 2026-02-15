#include "includes/backend/codegen/asm/x86_base_emitter.h"

X86BaseEmitter::X86BaseEmitter(const FuncTable& funcTable) :
    BaseEmitter(funcTable),
    m_nextTempStackOffset(0),
    m_nextVarStackOffset(0),
    m_selectLabelCounter(0)
{}

const char* X86BaseEmitter::GetSetccMnemonic(IROpcode opcode)
{
    switch (opcode)
    {
        case IROpcode::ICmpEq:  return "sete";
        case IROpcode::ICmpNe:  return "setne";
        case IROpcode::ICmpSlt: return "setl";
        case IROpcode::ICmpSle: return "setle";
        case IROpcode::ICmpSgt: return "setg";
        case IROpcode::ICmpSge: return "setge";
        case IROpcode::ICmpUlt: return "setb";
        case IROpcode::ICmpUle: return "setbe";
        case IROpcode::ICmpUgt: return "seta";
        case IROpcode::ICmpUge: return "setae";

        case IROpcode::FCmpEq:  return "sete";
        case IROpcode::FCmpNe:  return "setne";
        case IROpcode::FCmpLt:  return "setb";
        case IROpcode::FCmpLe:  return "setbe";
        case IROpcode::FCmpGt:  return "seta";
        case IROpcode::FCmpGe:  return "setae";

        default: return nullptr;
    }
}

std::string X86BaseEmitter::SelectLabelGenerator()
{
    int id = this->m_selectLabelCounter;
    this->m_selectLabelCounter++;
    return ".Lsel_" + std::to_string(id);
}

std::string X86BaseEmitter::GetSectionDirective(SectionType sectionType) const
{
    switch (sectionType)
    {
        case SectionType::TEXT:
            return "section .text\n";

        case SectionType::DATA:
            return "section .data\n";

        case SectionType::RODATA:
            // NASM uses .rdata on Windows, .rodata on ELF
            if (TargetArchitecture::GetOS() == OS::Windows)
            {
                return "section .rdata\n";
            }
            return "section .rodata\n";

        case SectionType::BSS:
            return "section .bss\n";

        default:
            return "";
    }
}

std::string X86BaseEmitter::GetDataDirective(int bytes) const
{
    switch (bytes)
    {
        case 1: return "db";
        case 2: return "dw";
        case 4: return "dd";
        case 8: return "dq";
        default: return "db";
    }
}

std::string X86BaseEmitter::GetReserveDirective(int bytes) const
{
    switch (bytes)
    {
        case 1:  return "resb";
        case 2:  return "resw";
        case 4:  return "resd";
        case 8:  return "resq";
        default: return "resb";
    }
}
