#pragma once

#include <sstream>
#include <string>

#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/asm/asm_common_types.h"
#include "includes/backend/codegen/asm/interface_target_emitter.h"
#include "includes/frontend/semantic_analysis/func_table.h"

class BaseEmitter : public ITargetEmitter
{
public:
    BaseEmitter(const FuncTable& funcTable);
    std::string Emit(const IRModule& module) override final;

protected:
    // header signaure for the file
    virtual std::string EmitHeader() = 0;
 
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
     */
    virtual void EmitEpilogue() = 0;

    // Convert given IR instruction into an assembly instruction
    virtual void EmitInstruction(IRInstruction* instruction, std::ostream& out) = 0;

    /// @brief The func emits a correct _start label that calls the main func and handles more stuff.
    virtual void EmitStart(std::ostream& out) = 0;

    /// @brief The func handles a return (puts in the correct register the return value).
    virtual void EmitReturn(std::ostream& out) = 0;

    // Gets sections name as a string e.g., section .text
    virtual std::string GetSectionDirective(SectionType sectionType) = 0;

    // Get global section for linker, will be avaiable between all files
    virtual std::string GetGlobalDirective(const std::string& symbol) = 0;

    // Converts operand into it's value in runtime:
    // e.g.,: IRConstantInt will evaluate as a it's value
    // Local variable will use stack offset [rbp - 4]
    // Globals will just use name [my_global]
    virtual std::string GetOperandAsm(const IRBaseValue* operand) = 0;

    /// @breif: Get .data keywords e.g., db, dw, etc..
    /// @param bytes: size of the variable in bytes
    virtual std::string GetDataDirective(int bytes) = 0;

    /// @breif: Get .bss keywords e.g., resb, resw, resd etc..
    /// @param bytes: size of the variable in bytes
    virtual std::string GetReserveDirective(int bytes) = 0;

private:
    const FuncTable& m_funcTable;
    const IRFunction* m_currentFunc;


    /// @brief Emits .data section (initialized globals)
    virtual void EmitDataSection(const IRModule& module, std::ostream& out) final;
    
    /// @brief Emits .bss section (uninitialized globals)
    virtual void EmitBssSection(const IRModule& module, std::ostream& out) final;
    
    /// @brief emits .text section (all functions)
    virtual void EmitTextSection(const IRModule& module, std::ostream& out) final;

    virtual void EmitFunction(const IRFunction* func, std::ostream& out) final;

    virtual void EmitBasicBlock(const BasicBlock* block, std::ostream& out) final;

    //------------------------//
    //                        //
    //    Helper Functions    //
    //                        //
    //------------------------//
    int GetTypeSize(std::shared_ptr<const IRType> type) const;
    std::string Indent() const;
};
