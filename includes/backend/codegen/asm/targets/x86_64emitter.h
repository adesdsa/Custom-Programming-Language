#pragma once

#include "includes/backend/codegen/asm/base_emitter.h"
#include "includes/shared/common/target_architecture.h"

class X86_64Emitter : public BaseEmitter
{
public:
    X86_64Emitter() = default;

private:
    std::unordered_map<IRBaseValue*, int> m_valueToStackOffset;
    size_t m_nextTempStackOffset;   // tracks top of ssa variable's frame
    size_t m_nextVarStackOffset;   // tracks top of normal func variable's frame

    // header signaure for the file
    virtual std::string EmitHeader() override final;
 
    /** 
      * my_function:
      * push rbp  ; Save old base pointer
      * mov rbp, rsp  ; Set new base pointer to current stack
      * sub rsp, 32  ; Reserve 32 bytes for local variables
    */
    virtual void EmitPrologue(const IRFunction* func, int frameSize, std::ostream& out) override final;

    /** 
      * mov rsp, rbp  ; Restore stack pointer
      * pop rbp  ; Restore old base pointer
     */
    virtual void EmitEpilogue(std::ostream& out) override final;

    // Convert given IR instruction into an assembly instruction
    virtual void EmitInstruction(IRInstruction* instruction, std::ostream& out) override final;

    /// @brief The func emits a correct _start label that calls the main func and handles more stuff.
    virtual void EmitStart(std::ostream& out) override final;

    /// @brief The func handles a return (puts in the correct register the return value).
    virtual void EmitReturn(std::ostream& out) override final;

    // Gets sections name as a string e.g., section .text
    virtual std::string GetSectionDirective(SectionType sectionType) override final;

    // Get global section for linker, will be avaiable between all files
    virtual std::string GetGlobalDirective(const std::string& symbol) override final;

    // Converts operand into it's value in runtime:
    // e.g.,: IRConstantInt will evaluate as a it's value
    // Local variable will use stack offset [rbp - 4]
    // Globals will just use name [my_global]
    virtual std::string GetOperandAsm(const IRBaseValue* operand) override final;

    /// @breif: Get .data keywords e.g., db, dw, etc..
    /// @param bytes: size of the variable in bytes
    virtual std::string GetDataDirective(int bytes) override final;

    /// @breif: Get .bss keywords e.g., resb, resw, resd etc..
    /// @param bytes: size of the variable in bytes
    virtual std::string GetReserveDirective(int bytes) override final;
};
