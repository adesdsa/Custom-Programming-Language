#pragma once

#include "includes/backend/codegen/asm/base_emitter.h"

#include <cstring>

namespace x86_32
{
    // operand indexing
    constexpr int SINGLE_OPERAND = 1;
    constexpr int SINGLE_OPERAND_INDEX = 0;
    constexpr int TWO_OPERANDS = 2;
    constexpr int TWO_OPERANDS_INDEX = 1;
    constexpr int THREE_OPERANDS = 3;
    constexpr int THREE_OPERANDS_INDEX = 2;

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

    constexpr std::string FPU_C2_FLAG = "0x04";
}

/**
 * Emits Intel x86 (32-bit) NASM assembly
 * Uses cdecl calling convention:
 *   - ALL arguments passed on stack (right-to-left)
 *   - Caller cleans up the stack
 *   - Return value in EAX
 * 
 * @see calling conventions at http://www.unixwiz.net/techtips/win32-callconv-asm.html
 * @see registers & flags at https://www.eecg.utoronto.ca/~amza/www.mindsec.com/files/x86regs.html
 */
class X86_Emitter : public BaseEmitter
{
public:
    explicit X86_Emitter(const FuncTable& funcTable);
    ~X86_Emitter() = default;

private:
    // stack offset tracking for locals/temps
    int m_nextVarStackOffset = 0;
    int m_nextTempStackOffset = 0;
    mutable std::unordered_map<const IRBaseValue*, int> m_valueToStackOffset;

    // Label counter for Select instruction
    int m_selectLabelCounter = 0;

    std::string EmitHeader() const override final;

    void EmitPrologue(
        const IRFunction* func,
        int frameSize,
        std::ostream& out
    ) override final;

    void EmitEpilogue(std::ostream& out) override final;

    void EmitInstruction(const IRInstruction* instruction, std::ostream& out) override final;

    void EmitStart(std::ostream& out) override final;

    std::string GetSectionDirective(SectionType sectionType) const override final;

    std::string GetGlobalDirective(const std::string& symbol) const override final;

    std::string GetOperandAsm(const IRBaseValue* operand, std::ostream& out) override final;

    std::string GetDataDirective(int bytes) const override final;
    std::string GetReserveDirective(int bytes) const override final;

    // allocate/retrieve stack slot for a value
    std::string slotOfInstr(const IRBaseValue* value);

    // generate unique labels for Select instruction
    std::string SelectLabelGenerator();

    // load an operand into an x87 FPU register
    void LoadOperandAsFloat(const IRBaseValue* operand, std::ostream& out);
};
