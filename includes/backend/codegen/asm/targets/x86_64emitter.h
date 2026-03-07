#pragma once

#include <vector>

#include "includes/backend/codegen/asm/x86_base_emitter.h"
#include "includes/shared/common/target_architecture.h"

namespace x86_64
{
    constexpr int FOUR_ARGUEMENTS = 4;
    constexpr int SIX_ARGUEMENTS = 6;
    constexpr int STACK_SLOT_SIZE = 8;
    constexpr int FIRST_STACK_ARG_OFFSET_LINUX = 16;
    constexpr int FIRST_STACK_ARG_OFFSET_WINDOWS = 48;
    constexpr int STACK_ALIGNMENT = 16;
    constexpr int WINDOWS_X64_SHADOW_SPACE = 32;

    inline const std::vector<std::string> linuxArgMapping = 
        {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

    inline const std::vector<std::string> windowsArgMapping =
        {"rcx", "rdx", "r8", "r9"};
}


/**
 * Emits Intel x86-64 (64-bit) assembly in NASM Intel syntax.
 *
 * This emitter targets the System V AMD64 ABI on Linux and
 * the Microsoft x64 ABI on Windows.
 *
 * Responsibilities:
 *  - Emit file headers and platform-specific entry points
 *  - Emit function prologues / epilogues with correct stack alignment
 *  - Lower custom IR instructions into x86-64 assembly
 *  - Handle register and stack-based operands
 *  - Manage SSA temporaries and local variable stack layout
 *
 * References:
 *  @see System V AMD64 ABI:
 *       https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf
 *
 *  @see Microsoft x64 calling convention:
 *       https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention
 *
 *  @see x86-64 registers & flags:
 *       https://www.felixcloutier.com/x86/
 */
class X86_64Emitter : public X86BaseEmitter
{
public:
    X86_64Emitter(const FuncTable& funcTable);

private:
    // header signature for the file
    std::string EmitHeader(const IRModule& module) const override final;

    /** 
      * my_function:
      * push rbp  ; Save old base pointer
      * mov rbp, rsp  ; Set new base pointer to current stack
      * sub rsp, 32  ; Reserve 32 bytes for local variables
    */
    void EmitPrologue(const IRFunction* func, int frameSize, std::ostream& out) override final;

    /** 
      * mov rsp, rbp  ; Restore stack pointer
      * pop rbp  ; Restore old base pointer
     */
    void EmitEpilogue(std::ostream& out) override final;

    // Convert given IR instruction into an assembly instruction
    void EmitInstruction(const IRInstruction* instruction, std::ostream& out) override final;

    /// @brief The func emits a correct _start label that calls the main func and handles more stuff.
    void EmitStart(const IRModule& module, std::ostream& out) override final;

    // Get global section for linker, will be available between all files
    std::string GetGlobalDirective(const std::string& symbol) const override final;

    // Converts operand into its value in runtime:
    // e.g., IRConstantInt will evaluate as its value
    // Local variable will use stack offset [rbp - 8]
    // Globals will just use name [my_global]
    std::string GetOperandAsm(const IRBaseValue* operand, std::ostream& out) override final;

    /// @brief The func turns an IRBaseValue into an asm value or memory location.
    /// @param value The value/memory location that we try to write into asm.
    /// @return A string that either holds a value or a memory location.
    std::string slotOfInstr(const IRBaseValue* value, std::ostream& out);

    /// @brief Picoc supports float+integer arithmetic+boolean thats why if one of them is float both need to be turned into one.
    /// @param operand The operand that is checked.
    /// @param xmmReg The special float register the value will be saved at.
    void LoadOperandAsFloat(const IRBaseValue* operand, const std::string& xmmReg, std::ostream& out);

    /// @brief A Select operand label generator (x86_64 prefixes with function name).
    std::string SelectLabelGenerator();

    void EmitExtraBssContent(const IRModule& module, std::ostream& out) const override;
};
