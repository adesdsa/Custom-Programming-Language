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
    /// @brief Emits .data section (initialized globals)
    void EmitDataSection(const IRModule& module, std::ostream& out);
    
    /// @brief Emits .bss section (uninitialized globals)
    void EmitBssSection(const IRModule& module, std::ostream& out);
    
    /// @brief emits .text section (all functions)
    void EmitTextSection(const IRModule& module, std::ostream& out);

    void EmitFunction(const IRFunction* func, std::ostream& out);

    void EmitBasicBlock(const BasicBlock* block, std::ostream& out);
    
    // Gets sections name as a string e.g., section .text
    virtual std::string GetSectionDirective(SectionType sectionType) = 0;

    // Get global section for linker, will be avaiable between all files
    virtual std::string GetGlobalDirective(const std::string& symbol) = 0;

    // header signaure for the file
    virtual std::string EmitHeader() = 0;
 
    /** 
      * my_function:
      * push rbp  ; Save old base pointer
      * mov rbp, rsp  ; Set new base pointer to current stack
      * sub rsp, 32  ; Reserve 32 bytes for local variables
    */
    virtual std::string EmitPrologue(const IRFunction* func, int frameSize, std::ostream& out) = 0;

    /** 
      * mov rsp, rbp  ; Restore stack pointer
      * pop rbp  ; Restore old base pointer
      * TODO: Check if `ret` should be here or it will be handled via the EmitInstructions
     */
    virtual std::string EmitEpilogue() = 0;

    // Convert given IR instruction into an assembly instruction
    virtual std::string EmitInstruction(IRInstruction* instruction, std::ostream& out) = 0;

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


    int GetTypeSize(std::shared_ptr<const IRType> type) const;
    std::string Indent() const;
};
