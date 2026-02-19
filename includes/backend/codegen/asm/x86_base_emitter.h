#pragma once

#include <unordered_map>

#include "includes/backend/codegen/asm/base_emitter.h"
#include "includes/backend/codegen/ir/ir_common_types.h"

// Constants shared across both x86 (32-bit) and x86_64 (64-bit) emitters.
namespace x86_common
{
    constexpr int SINGLE_OPERAND = 1;
    constexpr int SINGLE_OPERAND_INDEX = 0;
    constexpr int TWO_OPERANDS = 2;
    constexpr int TWO_OPERANDS_INDEX = 1;
    constexpr int THREE_OPERANDS = 3;
    constexpr int THREE_OPERANDS_INDEX = 2;
}

/**
 * Intermediate base class for all x86-family emitters (32-bit and 64-bit).
 *
 * Architecture-specific behaviour (registers, addressing, calling conventions)
 * remains in the concrete X86_Emitter and X86_64Emitter subclasses.
 */
class X86BaseEmitter : public BaseEmitter
{
public:
    explicit X86BaseEmitter(const FuncTable& funcTable);

protected:
    mutable std::unordered_map<const IRBaseValue*, int> m_valueToStackOffset;
    int m_nextTempStackOffset = 0;
    int m_nextVarStackOffset  = 0;
    int m_selectLabelCounter  = 0;

    /// @brief Map an integer/float comparison IROpcode to its SETcc mnemonic.
    /// @return The mnemonic string (e.g. "sete", "setl"), or nullptr for non-comparison opcodes.
    static const char* GetSetccMnemonic(IROpcode opcode);

    /// @brief Generate a unique label for Select (ternary) lowering.
    std::string SelectLabelGenerator();

    /// @brief Get section directive string e.g. "section .text\n"
    std::string GetSectionDirective(SectionType sectionType) const override final;

    /// @brief Get .data keyword for a given size e.g. "db", "dw", "dd", "dq"
    std::string GetDataDirective(int bytes) const override final;

    /// @brief Get .bss keyword for a given size e.g. "resb", "resw", "resd", "resq"
    std::string GetReserveDirective(int bytes) const override final;
};

