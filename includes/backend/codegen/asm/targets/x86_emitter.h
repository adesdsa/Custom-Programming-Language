#pragma once

#include "includes/backend/codegen/asm/base_emitter.h"

// skip saved EBP (4 bytes) + return address (4 bytes)
constexpr int BASE_OFFSET = 8;

// each argument takes 4 bytes in 32 bit
constexpr int ARGUMENT_BYTES = 4;

/**
    * Emits Intel x86 (32-bit)
    * @see calling convenstions at http://www.unixwiz.net/techtips/win32-callconv-asm.html
    * @see registers & flags at https://www.eecg.utoronto.ca/~amza/www.mindsec.com/files/x86regs.html
*/
class X86_Emitter : public BaseEmitter
{
public:
    X86_Emitter() = default;
    ~X86_Emitter() = default;

private:
    std::string EmitHeader() override final;

    void EmitPrologue(
        const IRFunction* func, 
        int frameSize, 
        std::ostream& out
    ) override final;

    void EmitEpilogue(std::ostream& out) override final;
    
    void EmitInstruction(IRInstruction* instruction, std::ostream& out) override final;

    void EmitStart(std::ostream& out) override final;

    std::string GetSectionDirective(SectionType sectionType) override final;

    std::string GetGlobalDirective(const std::string& symbol) override final;

    std::string GetOperandAsm(const IRBaseValue* operand) override final;

    std::string GetDataDirective(int bytes) override final;
    std::string GetReserveDirective(int bytes) override final;
};
