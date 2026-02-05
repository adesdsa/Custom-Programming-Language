#include "includes/backend/codegen/asm/targets/x86_64emitter.h"

std::string X86_64Emitter::EmitHeader()
{
    switch (TargetArchitecture::GetOS())
    {
        case OS::Linux:
            return
                "bits 64\n"
                "default rel\n"
                "section .text\n"
                "global _start\n\n";

        case OS::Windows:
            return
                "section .text\n"
                "global mainCRTStartup\n"
                "extern ExitProcess\n"
                "extern main\n\n";
        
        default:
            return "";
    }
}

void X86_64Emitter::EmitPrologue(
    const IRFunction* func,
    int frameSize,
    std::ostream& out)
{
    // Print function label.
    out << func->GetName() << ":\n";
    
    // Push old base pointer.
    out << "    push rbp\n";
    // Set new base pointer.
    out << "    mov rbp, rsp\n";

    int totalFrame = frameSize;

    switch (TargetArchitecture::GetOS())
    {
        case OS::Linux:
            // Align stack to 16 bytes for System V.
            totalFrame = (frameSize + 15) & ~15;
            break;

        case OS::Windows:
            // Add 32-byte shadow space required by MS x64 ABI.
            totalFrame = frameSize + 32;
            // Align to 16 bytes.
            totalFrame = (totalFrame + 15) & ~15;
            break;

        default:
            // fallback: just allocate frameSize.
            totalFrame = frameSize;
            break;
    }

    // Allocate stack space if needed
    if (totalFrame > 0)
    {
        out << "    sub rsp, " << totalFrame << "\n";
    }
    // Reset the SSA slot counter starting after locals.
    this->m_nextTempStackOffset = totalFrame;
    if (TargetArchitecture::GetOS() == OS::Linux)
    {
        this->m_nextVarStackOffset = 0;
    }
    else if (TargetArchitecture::GetOS() == OS::Windows)
    {
        this->m_nextVarStackOffset = 32; // first 32 bytes reserved for shadow space
    }

    out << "\n";
}

void X86_64Emitter::EmitEpilogue(std::ostream& out)
{
    out << "    mov rsp, rbp\n";  // Undo frame allocation.
    out << "    pop rbp\n";       // Restore old base pointer.
    out << "    ret\n";           // Return from function.
    this->m_valueToStackOffset.clear();
    this->m_nextTempStackOffset = 0;
}

void X86_64Emitter::EmitInstruction(IRInstruction* instr, std::ostream& out)
{
    auto opcode = instr->GetOpcode();

    // --- normal instruction operands ---
    IRBaseValue* source1 = instr->GetNumOperands() >= 1 ? instr->GetOperand(0) : nullptr;
    IRBaseValue* source2 = instr->GetNumOperands() >= 2 ? instr->GetOperand(1) : nullptr;
    IRBaseValue* source3 = instr->GetNumOperands() >= 3 ? instr->GetOperand(2) : nullptr;

    // --- SSA destination: the instruction itself ---
    auto slotOfInstr = [this](IRBaseValue* value) -> std::string
    {
        if (!value) return -1;

        auto it = m_valueToStackOffset.find(value);
        if (it != m_valueToStackOffset.end())
        {
            return "[rbp-" + std::to_string(it->second) + "]"; // already assigned
        }

        // allocate new stack slot
        int offset = this->m_nextTempStackOffset;
        this->m_nextTempStackOffset += 8;   // 8 bytes per slot
        m_valueToStackOffset[value] = offset;
        return "[rbp-" + std::to_string(offset) + "]";
    };

    // --- operand text: constants or stack slot ---
    auto operandText = [&](IRBaseValue* value) -> std::string
    {
        if (!value) return "0";
        if (auto castResult = dynamic_cast<IRConstantInt*>(value)) return std::to_string(castResult->GetValue());
        else if (auto castResult = dynamic_cast<IRConstantFloat*>(value))
        {
            // Store the float in the stack because you cant move a const value into register right away.
            float floatValue = castResult->GetValue();
            uint32_t bits;
            std::memcpy(&bits, &floatValue, sizeof(float)); // get IEEE 754 bits
            out << "    mov dword " << slotOfInstr(castResult) << ", 0x" 
                << std::hex << bits << "    ; float constant\n";
            return slotOfInstr(castResult); // return the stack slot
        }
        else if (auto castResult = dynamic_cast<IRConstantBool*>(value)) return std::to_string(castResult->GetValue());
        return slotOfInstr(value);
    };

    // --- label generator for Select ---
    auto makeLabel = [&]() -> std::string
    {
        static int localLabelCounter = 0;
        int id = (m_labelCounter++) + (localLabelCounter++ << 20);
        return ".Lsel_" + std::to_string(id);
    };

    switch (opcode)
    {
        // -----------------------
        // Integer arithmetic
        // -----------------------
        case IROpcode::Add:
        case IROpcode::Sub:
        case IROpcode::Mul:
        case IROpcode::SDiv:
        case IROpcode::UDiv:
        case IROpcode::SRem:
        case IROpcode::URem:
        {
            out << "    mov rax, " << operandText(source1) << "\n";

            switch (opcode)
            {
                case IROpcode::Add: out << "    add rax, " << operandText(source2) << "\n"; break;
                case IROpcode::Sub: out << "    sub rax, " << operandText(source2) << "\n"; break;
                case IROpcode::Mul: out << "    imul rax, " << operandText(source2) << "\n"; break;

                case IROpcode::SDiv:
                    out << "    cqo\n";
                    out << "    idiv " << operandText(source2) << "\n";
                    break;

                case IROpcode::UDiv:
                    out << "    xor rdx, rdx\n";
                    out << "    div " << operandText(source2) << "\n";
                    break;

                case IROpcode::SRem:
                    out << "    cqo\n";
                    out << "    idiv " << operandText(source2) << "\n";
                    out << "    mov rax, rdx\n"; // remainder
                    break;

                case IROpcode::URem:
                    out << "    xor rdx, rdx\n";
                    out << "    div " << operandText(source2) << "\n";
                    out << "    mov rax, rdx\n"; // remainder
                    break;

                default: break;
            }

            out << "    mov " << slotOfInstr(instr) << ", rax\n";
            break;
        }

        // -----------------------
        // Bitwise
        // -----------------------
        case IROpcode::And: out << "    mov rax, " << operandText(source1) << "\n"
                                << "    and rax, " << operandText(source2) << "\n"
                                << "    mov " << slotOfInstr(instr) << ", rax\n"; break;

        case IROpcode::Or:  out << "    mov rax, " << operandText(source1) << "\n"
                                << "    or rax, " << operandText(source2) << "\n"
                                << "    mov " << slotOfInstr(instr) << ", rax\n"; break;

        case IROpcode::Xor: out << "    mov rax, " << operandText(source1) << "\n"
                                << "    xor rax, " << operandText(source2) << "\n"
                                << "    mov " << slotOfInstr(instr) << ", rax\n"; break;

        case IROpcode::Shl: out << "    mov rax, " << operandText(source1) << "\n"
                                << "    mov cl, " << operandText(source2) << "\n"
                                << "    shl rax, cl\n"
                                << "    mov " << slotOfInstr(instr) << ", rax\n"; break;

        case IROpcode::LShr: out << "    mov rax, " << operandText(source1) << "\n"
                                 << "    mov cl, " << operandText(source2) << "\n"
                                 << "    shr rax, cl\n"
                                 << "    mov " << slotOfInstr(instr) << ", rax\n"; break;

        case IROpcode::AShr: out << "    mov rax, " << operandText(source1) << "\n"
                                 << "    mov cl, " << operandText(source2) << "\n"
                                 << "    sar rax, cl\n"
                                 << "    mov " << slotOfInstr(instr) << ", rax\n"; break;

        // -----------------------
        // Integer comparisons
        // -----------------------
        case IROpcode::ICmpEq: case IROpcode::ICmpNe:
        case IROpcode::ICmpSlt: case IROpcode::ICmpSle:
        case IROpcode::ICmpSgt: case IROpcode::ICmpSge:
        case IROpcode::ICmpUlt: case IROpcode::ICmpUle:
        case IROpcode::ICmpUgt: case IROpcode::ICmpUge:
        {
            out << "    mov rax, " << operandText(source1) << "\n";
            out << "    cmp rax, " << operandText(source2) << "\n";

            const char* setInstr = "sete";
            switch (opcode)
            {
                case IROpcode::ICmpEq: setInstr="sete"; break;
                case IROpcode::ICmpNe: setInstr="setne"; break;
                case IROpcode::ICmpSlt: setInstr="setl"; break;
                case IROpcode::ICmpSle: setInstr="setle"; break;
                case IROpcode::ICmpSgt: setInstr="setg"; break;
                case IROpcode::ICmpSge: setInstr="setge"; break;
                case IROpcode::ICmpUlt: setInstr="setb"; break;
                case IROpcode::ICmpUle: setInstr="setbe"; break;
                case IROpcode::ICmpUgt: setInstr="seta"; break;
                case IROpcode::ICmpUge: setInstr="setae"; break;
                default: break;
            }

            out << "    " << setInstr << " al\n"
                << "    movzx rax, al\n"
                << "    mov " << slotOfInstr(instr) << ", rax\n";
            break;
        }

        // -----------------------
        // Floating-point arithmetic (SSE)
        // -----------------------
        case IROpcode::FAdd: case IROpcode::FSub:
        case IROpcode::FMul: case IROpcode::FDiv:
        case IROpcode::FRem:
        {
            out << "    movss xmm0, dword " << operandText(source1) << "\n";

            switch (opcode)
            {
                case IROpcode::FAdd:
                    out << "    movss xmm1, dword " << operandText(source2) << "\n";
                    out << "    addss xmm0, xmm1\n";
                    out << "    movss dword " << slotOfInstr(instr) << ", xmm0\n";
                    break;

                case IROpcode::FSub:
                    out << "    movss xmm1, dword " << operandText(source2) << "\n";
                    out << "    subss xmm0, xmm1\n";
                    out << "    movss dword " << slotOfInstr(instr) << ", xmm0\n";
                    break;

                case IROpcode::FMul:
                    out << "    movss xmm1, dword " << operandText(source2) << "\n";
                    out << "    mulss xmm0, xmm1\n";
                    out << "    movss dword " << slotOfInstr(instr) << ", xmm0\n";
                    break;

                case IROpcode::FDiv:
                    out << "    movss xmm1, dword " << operandText(source2) << "\n";
                    out << "    divss xmm0, xmm1\n";
                    out << "    movss dword " << slotOfInstr(instr) << ", xmm0\n";
                    break;

                case IROpcode::FRem:
                    out << "    movss xmm1, dword " << operandText(source2) << "\n";
                    out << "    divss xmm0, xmm1\n";
                    out << "    roundss xmm0, xmm0, 1\n";  // floor(a/b)
                    out << "    mulss xmm0, xmm1\n";
                    out << "    movss xmm2, dword " << operandText(source1) << "\n";
                    out << "    subss xmm2, xmm0\n";
                    out << "    movss dword " << slotOfInstr(instr) << ", xmm2\n";
                    break;

                default:
                    break;
            }
        }

        // -----------------------
        // Floating-point comparisons
        // -----------------------
        case IROpcode::FCmpEq: case IROpcode::FCmpNe:
        case IROpcode::FCmpLt: case IROpcode::FCmpLe:
        case IROpcode::FCmpGt: case IROpcode::FCmpGe:
        {
            out << "    movss xmm0, dword " << slotOfInstr(source1) << "\n";
            out << "    movss xmm1, dword " << slotOfInstr(source2) << "\n";
            out << "    ucomiss xmm0, xmm1\n";

            const char* setInstr="sete";
            switch(opcode){
                case IROpcode::FCmpEq: setInstr="sete"; break;
                case IROpcode::FCmpNe: setInstr="setne"; break;
                case IROpcode::FCmpLt: setInstr="setb"; break;
                case IROpcode::FCmpLe: setInstr="setbe"; break;
                case IROpcode::FCmpGt: setInstr="seta"; break;
                case IROpcode::FCmpGe: setInstr="setae"; break;
                default: break;
            }

            out << "    " << setInstr << " al\n"
                << "    movzx rax, al\n"
                << "    mov " << slotOfInstr(instr) << ", rax\n";
            break;
        }

        // -----------------------
        // Memory
        // -----------------------
        case IROpcode::Load:
            out << "    mov rax, " << operandText(source1) << "\n"
                << "    mov rax, [rax]\n"
                << "    mov " << slotOfInstr(instr) << ", rax\n";
            break;

        case IROpcode::Store:
            out << "    mov rax, " << operandText(source1) << "\n"
                << "    mov rdx, " << operandText(source2) << "\n"
                << "    mov [rdx], rax\n";
            break;

        case IROpcode::Alloca:
            // Allocate a new stack slot for this instruction
            int offset = this->m_nextVarStackOffset;
            this->m_nextVarStackOffset += 8;
            this->m_valueToStackOffset[instr] = offset;
            break;

        // -----------------------
        // Control flow
        // -----------------------
        case IROpcode::Br:
            out << "    jmp " << source1 << "\n";
            break;

        case IROpcode::CondBr:
            out << "    mov rax, " << operandText(source1) << "\n"
                << "    cmp rax, 0\n"
                << "    jne " << source2 << "\n"
                << "    jmp " << source3 << "\n";
            break;

        case IROpcode::Ret:
            if(source1) out << "    mov rax, " << operandText(source1) << "\n";
            out << "    jmp .func_epilogue\n";
            break;

        case IROpcode::Call:
            out << "    call " << instr->GetCalledFunctionName() << "\n";
            out << "    mov " << slotOfInstr(instr) << ", rax\n";
            break;

        case IROpcode::Select:
        {
            std::string lblTrue = makeLabel();
            std::string lblEnd = makeLabel();
            out << "    mov rax, " << operandText(source1) << "\n"
                << "    cmp rax, 0\n"
                << "    jne " << lblTrue << "\n"
                << "    mov rax, " << operandText(source2) << "\n"
                << "    jmp " << lblEnd << "\n"
                << lblTrue << ":\n"
                << "    mov rax, " << operandText(source3) << "\n"
                << lblEnd << ":\n"
                << "    mov " << slotOfInstr(instr) << ", rax\n";
            break;
        }

        default:
            break;
    }
}

