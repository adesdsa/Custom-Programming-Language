#pragma once

#include <sstream>
#include <string>

#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/asm/asm_common_types.h"
#include "includes/backend/codegen/asm/interface_target_emitter.h"
#include "includes/frontend/semantic_analysis/func_table.h"
#include "includes/shared/common/target_architecture.h"

class BaseEmitter : public ITargetEmitter
{
public:
    BaseEmitter(const FuncTable& funcTable);
    std::string Emit(const IRModule& module) override final;

protected:
    const FuncTable& m_funcTable;
    const IRFunction* m_currentFunc;
    
    // header signature for the file
    virtual std::string EmitHeader(const IRModule& module) const = 0;

    /** 
      * my_function:
      * push rbp  ; Save old base pointer
      * mov rbp, rsp  ; Set new base pointer to current stack
      * sub rsp, 32  ; Reserve 32 bytes for local variables
    */
    virtual void EmitPrologue(const IRFunction* func, int frameSize, std::ostream& out) = 0;

    /** 
      * mov rsp, rbp  ; Restore stack pointer
      * pop rbp  ; Restore old base pointer
      * ret
     */
    virtual void EmitEpilogue(std::ostream& out) = 0;

    // Convert given IR instruction into an assembly instruction
    virtual void EmitInstruction(const IRInstruction* instruction, std::ostream& out) = 0;

    /// @brief The func emits a correct _start label that calls the main func and handles more stuff.
    virtual void EmitStart(const IRModule& module, std::ostream& out) = 0;

    // Gets sections name as a string e.g., section .text
    virtual std::string GetSectionDirective(SectionType sectionType) const = 0;

    // Get global section for linker, will be available between all files
    virtual std::string GetGlobalDirective(const std::string& symbol) const = 0;

    // Converts operand into its value in runtime:
    // e.g., IRConstantInt will evaluate as its value
    // Local variable will use stack offset [rbp - 4] or [ebp - 4]
    // Globals will just use name [my_global]
    virtual std::string GetOperandAsm(const IRBaseValue* operand, std::ostream& out) = 0;

    /// @brief Get .data keywords e.g., db, dw, etc..
    /// @param bytes size of the variable in bytes
    virtual std::string GetDataDirective(int bytes) const = 0;

    /// @brief Get .bss keywords e.g., resb, resw, resd etc..
    /// @param bytes size of the variable in bytes
    virtual std::string GetReserveDirective(int bytes) const = 0;

private:
    /** 
     * Emits global/extern directives for cross-module linking:
     *  1. For each exported function/variable: emit  global _funcname
     *  2. For each imported function/variable: emit  extern _funcname
     */
    void EmitExternAndGlobalDirectives(const IRModule& module, std::ostream& out);

    /// @brief Emits .data section (initialized globals). Skips extern variables.
    void EmitDataSection(const IRModule& module, std::ostream& out);
    
    /// @brief Emits .bss section (uninitialized globals). Skips extern variables.
    void EmitBssSection(const IRModule& module, std::ostream& out);
    
    /// @brief emits .text section (all functions)
    void EmitTextSection(const IRModule& module, std::ostream& out);

    void EmitFunction(const IRFunction* func, std::ostream& out);

    void EmitBasicBlock(const BasicBlock* block, std::ostream& out);

    /// @brief Returns the platform-mangled version of a symbol name.
    /// For example, 32-bit Windows prefixes C symbols with '_'.
    /// Override in target emitters that need name mangling.
    virtual std::string MangledName(const std::string& name) const;

    //------------------------//
    //                        //
    //    Helper Functions    //
    //                        //
    //------------------------//
    int GetTypeSize(std::shared_ptr<const IRType> type) const;
    std::string Indent() const;
};
