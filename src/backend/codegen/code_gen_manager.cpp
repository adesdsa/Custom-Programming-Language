#include "includes/backend/codegen/code_gen_manager.h"

CodeGenManager::CodeGenManager(const DecoratedAST& decoratedAST) :
    m_decoratedAST(decoratedAST),
    m_errorManager(ErrorManager::GetInstance())
{}

void CodeGenManager::MainFlow()
{
    IRGenerator irGenerator(this->m_decoratedAST);
    IRModule& irProgram = irGenerator.MainFlow();

    OptimizationManager optimizationManager;
    optimizationManager.MainFlow(irProgram);

    /*std::string asmContent;
    if (TargetArchitecture::Is32Bit())
    {
        X86_Emitter emitter(this->m_decoratedAST.funcTable);
        asmContent = emitter.Emit(irProgram);
    }
    else if (TargetArchitecture::Is64Bit())
    {
        X86_64Emitter emitter(this->m_decoratedAST.funcTable);
        asmContent = emitter.Emit(irProgram);
    }
    else
    {
        this->m_errorManager.StoreError(CompileTimeError(ErrorTypes::INTERNAL_ERROR,
            "Unsupported OS for backend",
            0,
            0,
            "")
        );
        return;
    }*/

    std::string asmContent = R"asm(
global main
section .text

main:
    ; Reserve 8 bytes for local variable x
    sub rsp, 8

    ; int x = 1
    mov dword [rsp], 1

    ; Return 0
    xor eax, eax

    ; Restore stack
    add rsp, 8
    ret
)asm";
    Assembler assembler(this->m_errorManager);
    std::filesystem::path objPath = assembler.Assemble(asmContent);
    if (objPath.empty()) this->m_errorManager.HandleErrors();

    Linker linker(this->m_errorManager);
    std::filesystem::path exePath = linker.Link(objPath);
    /// @todo handle output
}
