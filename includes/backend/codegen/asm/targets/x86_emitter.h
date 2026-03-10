#pragma once

#include "includes/backend/codegen/asm/x86_base_emitter.h"

#include <cstring>

namespace x86_32
{
    // 32-bit stack slot size
    constexpr int STACK_SLOT_SIZE = 4;

    // stack alignment
    constexpr int STACK_ALIGNMENT = 4;
    // via bitwise operator alignment of 4 works like so: (n + 3) & ~3
    constexpr int STACK_ALIGNMENT_BIT_WISE = 3;

    // Skip saved EBP (4 bytes) + return address (4 bytes)
    constexpr int BASE_OFFSET = 8;

    // Each argument takes 4 bytes in 32-bit cdecl
    constexpr int ARGUMENT_SIZE = 4;
}

/**
 * Emits Intel x86 (32-bit) NASM assembly
 * 
 * References:
 * 
 * Calling Conventions:
 *      @see http://www.unixwiz.net/techtips/win32-callconv-asm.html
 * 
 * Registers & Flags: 
 *      @see https://www.eecg.utoronto.ca/~amza/www.mindsec.com/files/x86regs.html
 */
class X86_Emitter : public X86BaseEmitter
{
public:
    explicit X86_Emitter(const FuncTable& funcTable);
    ~X86_Emitter() = default;

private:
    std::string EmitHeader(const IRModule& module) const override final;

    void EmitPrologue(
        const IRFunction* func,
        int frameSize,
        std::ostream& out
    ) override final;

    void EmitEpilogue(std::ostream& out) override final;

    void EmitInstruction(const IRInstruction* instruction, std::ostream& out) override final;

    void EmitStart(const IRModule& module, std::ostream& out) override final;

    void EmitAfterFunctionLabel(const IRFunction* func, std::ostream& out) override final;

    std::string MangledName(const std::string& name) const override final;

    std::string GetGlobalDirective(const std::string& symbol) const override final;

    std::string GetOperandAsm(const IRBaseValue* operand, std::ostream& out) override final;

    // allocate/retrieve stack slot for a value
    std::string slotOfInstr(const IRBaseValue* value);

    // load an operand into an SSE XMM register
    void LoadOperandAsFloat(const IRBaseValue* operand, const std::string& xmmReg, std::ostream& out);

    void EmitExtraBssContent(const IRModule& module, std::ostream& out) const override;
};
