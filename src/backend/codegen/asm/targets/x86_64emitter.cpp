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
    
    // Push old base pointer and set new one.
    out << "    push rbp\n";
    out << "    mov rbp, rsp\n";

    // Calculate total frame = aligned frameSize (do NOT add caller shadow space)
    int totalFrame = frameSize;
    switch (TargetArchitecture::GetOS())
    {
        case OS::Linux:
            totalFrame = (frameSize + 15) & ~15;
            break;

        case OS::Windows:
            // Do NOT add the 32-byte shadow space here (caller provides it).
            totalFrame = (frameSize + 15) & ~15;
            break;

        default:
            totalFrame = (frameSize + 15) & ~15;
            break;
    }

    // Allocate stack space if needed
    if (totalFrame > 0)
    {
        out << "    sub rsp, " << totalFrame << "\n";
    }

    // next temp slot starts right after the allocated locals (offset from rbp)
    this->m_nextTempStackOffset = totalFrame;

    // For addressing incoming args on Windows, many backends treat shadow-space as
    // existing at rbp+16..rbp+47; so set nextVar offset base for Alloca if you want:
    if (TargetArchitecture::GetOS() == OS::Linux)
    {
        this->m_nextVarStackOffset = 0;
    }
    else if (TargetArchitecture::GetOS() == OS::Windows)
    {
        // We don't allocate the shadow space, but logically treat it as present
        // above saved RBP for computing argument addresses.
        this->m_nextVarStackOffset = 0;
    }

    this->m_currentFunc = func;
    out << "\n";
}

void X86_64Emitter::EmitEpilogue(std::ostream& out)
{
    out << "    mov rsp, rbp\n";  // Undo frame allocation.
    out << "    pop rbp\n";       // Restore old base pointer.
    out << "    ret\n";           // Return from function.
    this->m_valueToStackOffset.clear();
    this->m_nextTempStackOffset = 0;
    this->m_currentFunc = nullptr;
}

void X86_64Emitter::EmitInstruction(IRInstruction* instr, std::ostream& out)
{
    int offset = 0;
    int numOperands = 0;
    std::vector<IRBaseValue*> operands;
    auto opcode = instr->GetOpcode();

    // --- normal instruction operands ---
    IRBaseValue* source1 = instr->GetNumOperands() >= 1 ? instr->GetOperand(0) : nullptr;
    IRBaseValue* source2 = instr->GetNumOperands() >= 2 ? instr->GetOperand(1) : nullptr;
    IRBaseValue* source3 = instr->GetNumOperands() >= 3 ? instr->GetOperand(2) : nullptr;

    // --- label generator for Select ---
    auto makeLabel = [&]() -> std::string
    {
        static int localLabelCounter = 0;
        int id = (localLabelCounter++) + (localLabelCounter++ << 20);
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
            out << "    mov rax, " << GetOperandAsm(source1, out) << "\n";

            switch (opcode)
            {
                case IROpcode::Add: out << "    add rax, " << GetOperandAsm(source2, out) << "\n"; break;
                case IROpcode::Sub: out << "    sub rax, " << GetOperandAsm(source2, out) << "\n"; break;
                case IROpcode::Mul: out << "    imul rax, " << GetOperandAsm(source2, out) << "\n"; break;

                case IROpcode::SDiv:
                    out << "    cqo\n";
                    out << "    idiv " << GetOperandAsm(source2, out) << "\n";
                    break;

                case IROpcode::UDiv:
                    out << "    xor rdx, rdx\n";
                    out << "    div " << GetOperandAsm(source2, out) << "\n";
                    break;

                case IROpcode::SRem:
                    out << "    cqo\n";
                    out << "    idiv " << GetOperandAsm(source2, out) << "\n";
                    out << "    mov rax, rdx\n"; // remainder
                    break;

                case IROpcode::URem:
                    out << "    xor rdx, rdx\n";
                    out << "    div " << GetOperandAsm(source2, out) << "\n";
                    out << "    mov rax, rdx\n"; // remainder
                    break;

                default: break;
            }

            out << "    mov " << GetOperandAsm(instr, out) << ", rax\n";
            break;
        }

        // -----------------------
        // Bitwise
        // -----------------------
        case IROpcode::And: out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                                << "    and rax, " << GetOperandAsm(source2, out) << "\n"
                                << "    mov " << GetOperandAsm(instr, out) << ", rax\n"; break;

        case IROpcode::Or:  out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                                << "    or rax, " << GetOperandAsm(source2, out) << "\n"
                                << "    mov " << GetOperandAsm(instr, out) << ", rax\n"; break;

        case IROpcode::Xor: out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                                << "    xor rax, " << GetOperandAsm(source2, out) << "\n"
                                << "    mov " << GetOperandAsm(instr, out) << ", rax\n"; break;

        case IROpcode::Shl: out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                                << "    mov cl, " << GetOperandAsm(source2, out) << "\n"
                                << "    shl rax, cl\n"
                                << "    mov " << GetOperandAsm(instr, out) << ", rax\n"; break;

        case IROpcode::LShr: out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                                 << "    mov cl, " << GetOperandAsm(source2, out) << "\n"
                                 << "    shr rax, cl\n"
                                 << "    mov " << GetOperandAsm(instr, out) << ", rax\n"; break;

        case IROpcode::AShr: out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                                 << "    mov cl, " << GetOperandAsm(source2, out) << "\n"
                                 << "    sar rax, cl\n"
                                 << "    mov " << GetOperandAsm(instr, out) << ", rax\n"; break;

        // -----------------------
        // Integer comparisons
        // -----------------------
        case IROpcode::ICmpEq: case IROpcode::ICmpNe:
        case IROpcode::ICmpSlt: case IROpcode::ICmpSle:
        case IROpcode::ICmpSgt: case IROpcode::ICmpSge:
        case IROpcode::ICmpUlt: case IROpcode::ICmpUle:
        case IROpcode::ICmpUgt: case IROpcode::ICmpUge:
        {
            out << "    mov rax, " << GetOperandAsm(source1, out) << "\n";
            out << "    cmp rax, " << GetOperandAsm(source2, out) << "\n";

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
                << "    mov " << GetOperandAsm(instr, out) << ", rax\n";
            break;
        }

        // -----------------------
        // Floating-point arithmetic (SSE)
        // -----------------------
        case IROpcode::FAdd: case IROpcode::FSub:
        case IROpcode::FMul: case IROpcode::FDiv:
        case IROpcode::FRem:
        {
            // Load operands into XMM registers
            LoadOperandAsFloat(source1, "xmm0", out);
            LoadOperandAsFloat(source2, "xmm1", out);

            switch (opcode)
            {
                case IROpcode::FAdd:
                    out << "    addss xmm0, xmm1\n";
                    break;

                case IROpcode::FSub:
                    out << "    subss xmm0, xmm1\n";
                    break;

                case IROpcode::FMul:
                    out << "    mulss xmm0, xmm1\n";
                    break;

                case IROpcode::FDiv:
                    out << "    divss xmm0, xmm1\n";
                    break;

                case IROpcode::FRem:
                {
                    // xmm0 = a, xmm1 = b (already loaded)
                    out << "    divss xmm0, xmm1\n";             // a / b
                    out << "    roundss xmm0, xmm0, 1    ; floor(a/b)\n";
                    out << "    mulss xmm0, xmm1\n";             // floor(a/b) * b

                    // Load original a into xmm2 using correct size
                    if (source1->GetType() == IRType::GetFloat64()) {
                        // double precision
                        out << "    movsd xmm2, qword " << GetOperandAsm(source1, out) << "\n";
                    } else {
                        // assume float32 or int (we already converted ints earlier when loading operands)
                        // if source1 was an integer, GetOperandAsm may be a slot with int — convert it:
                        if (source1->GetType() == IRType::GetFloat64()) {
                            out << "    movss xmm2, dword " << GetOperandAsm(source1, out) << "\n";
                        } else {
                            out << "    mov eax, " << GetOperandAsm(source1, out) << "\n";
                            out << "    cvtsi2ss xmm2, eax\n";
                        }
                    }

                    out << "    subss xmm2, xmm0\n";
                    out << "    movss dword " << GetOperandAsm(instr, out) << ", xmm2\n";
                    return; // finished (keeps behavior consistent with your earlier code)
                }
            }

            // Store result back
            out << "    movss dword " << GetOperandAsm(instr, out) << ", xmm0\n";
            break;
        }

        // -----------------------
        // Floating-point comparisons
        // -----------------------
        case IROpcode::FCmpEq: case IROpcode::FCmpNe:
        case IROpcode::FCmpLt: case IROpcode::FCmpLe:
        case IROpcode::FCmpGt: case IROpcode::FCmpGe:
        {
            // Load operands as float
            LoadOperandAsFloat(source1, "xmm0", out);
            LoadOperandAsFloat(source2, "xmm1", out);

            // Compare
            out << "    ucomiss xmm0, xmm1\n";

            const char* setInstr = "sete";
            switch(opcode){
                case IROpcode::FCmpEq: setInstr="sete"; break;
                case IROpcode::FCmpNe: setInstr="setne"; break;
                case IROpcode::FCmpLt: setInstr="setb"; break;
                case IROpcode::FCmpLe: setInstr="setbe"; break;
                case IROpcode::FCmpGt: setInstr="seta"; break;
                case IROpcode::FCmpGe: setInstr="setae"; break;
                default: break;
            }

            // Store boolean result (0 or 1)
            out << "    " << setInstr << " al\n";
            out << "    movzx rax, al\n";
            out << "    mov " << GetOperandAsm(instr, out) << ", rax\n";
            break;
        }

        // -----------------------
        // Memory
        // -----------------------
        case IROpcode::Load:
        {
            auto srcAsm = GetOperandAsm(source1, out);
            // If srcAsm is a memory operand like "[rbp-8]", load directly.
            if (!srcAsm.empty() && srcAsm.front() == '[') {
                out << "    mov rax, " << srcAsm << "\n";
            } else {
                // srcAsm is a register or immediate that contains an address; deref it.
                out << "    mov rax, [" << srcAsm << "]\n";
            }
            out << "    mov " << GetOperandAsm(instr, out) << ", rax\n";
            break;
        }

        case IROpcode::Store:
        {
            auto dstAsm = GetOperandAsm(source2, out); // destination address or memory operand
            // Load value to store into rax
            out << "    mov rax, " << GetOperandAsm(source1, out) << "\n";

            if (!dstAsm.empty() && dstAsm.front() == '[') {
                // destination is a memory operand: store directly
                out << "    mov " << dstAsm << ", rax\n";
            } else {
                // destination is an address in a register/immediate: deref into memory
                out << "    mov [" << dstAsm << "], rax\n";
            }
            break;
        }

        case IROpcode::Alloca:
            // Allocate a new stack slot for this instruction
            offset = this->m_nextVarStackOffset;
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
            out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                << "    cmp rax, 0\n"
                << "    jne " << source2 << "\n"
                << "    jmp " << source3 << "\n";
            break;

        case IROpcode::Ret:
            if(source1) out << "    mov rax, " << GetOperandAsm(source1, out) << "\n";
            out << "    jmp .func_epilogue\n";
            break;

        case IROpcode::Call:
            operands = instr->GetOperands();
            numOperands = static_cast<int>(operands.size());

            if (TargetArchitecture::GetOS() == OS::Linux)
            {
                int extraArgs = numOperands > 6 ? numOperands - 6 : 0;
                if (extraArgs % 2 != 0) out << "    sub rsp, 8    ; align stack\n";

                // push extra args right-to-left
                for (int i = numOperands - 1; i >= 6; --i)
                    out << "    push " << GetOperandAsm(operands[i], out) << "\n";

                // move register args
                for (int i = 0; i < std::min(6, numOperands); ++i)
                    out << "    mov " << x86_64::linuxArgMapping[i]
                        << ", " << GetOperandAsm(operands[i], out) << "\n";

                out << "    call " << instr->GetCalledFunction()->GetName() << "\n";
                out << "    mov " << GetOperandAsm(instr, out) << ", rax\n";

                if (extraArgs > 0) out << "    add rsp, " << (extraArgs * 8) << "\n";
                if (extraArgs % 2 != 0) out << "    add rsp, 8    ; restore alignment\n";
            }
            else if (TargetArchitecture::GetOS() == OS::Windows)
            {
                int extraArgs = numOperands > 4 ? numOperands - 4 : 0;

                // Reserve 32-byte shadow (always required by caller).
                // We'll also allocate space for extra pushed args and alignment in one reservation,
                // so push is not needed: we will write extras into that space or do pushes but then cleaning is simpler.
                // Simpler approach (preserve your push-based style): reserve shadow+possible padding first.
                int pad = (extraArgs % 2 != 0) ? 8 : 0; // keep 16-bytes alignment rule
                int reserve = 32 + pad; // keep space for shadow + alignment
                out << "    sub rsp, " << reserve << "    ; reserve shadow + padding\n";

                // Push extra args (right-to-left) AFTER reserve (they will grow below current rsp)
                for (int i = numOperands - 1; i >= 4; --i)
                    out << "    push " << GetOperandAsm(operands[i], out) << "\n";

                // Move first 4 args into registers
                for (int i = 0; i < std::min(4, numOperands); ++i)
                    out << "    mov " << x86_64::windowsArgMapping[i]
                        << ", " << GetOperandAsm(operands[i], out) << "\n";

                // Call
                out << "    call " << instr->GetCalledFunction()->GetName() << "\n";
                out << "    mov " << GetOperandAsm(instr, out) << ", rax\n";

                // Cleanup pushed args
                if (extraArgs > 0)
                    out << "    add rsp, " << (extraArgs * 8) << "\n";

                // Restore reserved shadow + padding
                out << "    add rsp, " << reserve << "    ; restore shadow + padding\n";
            }
            else
            {
                out << "    call " << instr->GetCalledFunction()->GetName() << "\n";
                out << "    mov " << GetOperandAsm(instr, out) << ", rax\n";
            }
            break;

        case IROpcode::Select:
        {
            std::string lblTrue = makeLabel();
            std::string lblEnd = makeLabel();
            out << "    mov rax, " << GetOperandAsm(source1, out) << "\n"
                << "    cmp rax, 0\n"
                << "    jne " << lblTrue << "\n"
                << "    mov rax, " << GetOperandAsm(source2, out) << "\n"
                << "    jmp " << lblEnd << "\n"
                << lblTrue << ":\n"
                << "    mov rax, " << GetOperandAsm(source3, out) << "\n"
                << lblEnd << ":\n"
                << "    mov " << GetOperandAsm(instr, out) << ", rax\n";
            break;
        }

        default:
            break;
    }
}

void X86_64Emitter::EmitStart(std::ostream& out)
{
    if (TargetArchitecture::GetOS() == OS::Linux)
    {
        out << "global _start\n";
        out << "extern main\n";
        out << "extern exit\n\n";

        out << "section .text\n";
        out << "_start:\n";

        // Kernel enters with RSP % 16 == 8
        // Align before calling C code
        out << "    and rsp, -16\n";

        // call main()
        out << "    call main\n";

        // exit(main_return)
        out << "    mov rdi, rax\n";
        out << "    call exit\n";

        out << "    hlt\n";
    }
    else if (TargetArchitecture::GetOS() == OS::Windows)
    {
        out << "global mainCRTStartup\n";
        out << "extern main\n";
        out << "extern ExitProcess\n\n";

        out << "section .text\n";
        out << "mainCRTStartup:\n";

        // Windows x64 ABI:
        // - Stack is 16-byte aligned *before call*
        // - 32 bytes of shadow space required
        out << "    sub rsp, 40\n"; // 32 shadow + 8 align

        // call main()
        out << "    call main\n";

        // ExitProcess(main_return)
        out << "    mov ecx, eax\n";
        out << "    call ExitProcess\n";

        out << "    hlt\n";
    }
}

std::string X86_64Emitter::GetSectionDirective(SectionType sectionType)
{
    switch (sectionType)
    {
    case SectionType::TEXT:
        return "section .text";

    case SectionType::DATA:
        return "section .data";

    case SectionType::RODATA:
        // NASM uses .rdata on Windows, .rodata on ELF
        if (TargetArchitecture::GetOS() == OS::Windows) return "section .rdata";
        return "section .rodata";

    case SectionType::BSS:
        return "section .bss";

    default:
        return "";
    }
}

std::string X86_64Emitter::GetGlobalDirective(const std::string& symbol)
{
    return "global " + symbol;
}

std::string X86_64Emitter::GetDataDirective(int bytes)
{
    switch (bytes)
    {
    case 1: return "db";
    case 2: return "dw";
    case 4: return "dd";
    case 8: return "dq";
    default:
        // Fallback.
        return "db";
    }
}

std::string X86_64Emitter::GetReserveDirective(int bytes)
{
    switch (bytes)
    {
    case 1: return "resb";
    case 2: return "resw";
    case 4: return "resd";
    case 8: return "resq";
    default: return "resb";
    }
}

std::string X86_64Emitter::GetOperandAsm(IRBaseValue* operand, std::ostream& out)
{
    if (!operand) return "0";

    if (auto c = dynamic_cast<IRConstantInt*>(operand)) {
        return std::to_string(c->GetValue());
    }
    else if (auto c = dynamic_cast<IRConstantFloat*>(operand))
    {
        // allocate a slot once and write the constant there
        std::string slot = slotOfInstr(c, out);
        // write constant bits into slot (single-precision assumed)
        float floatValue = c->GetValue();
        uint32_t bits;
        std::memcpy(&bits, &floatValue, sizeof(float));
        out << "    mov dword " << slot << ", 0x" << std::hex << bits << "    ; float constant\n";
        // return the slot (address expression)
        return slot;
    }
    else if (auto c = dynamic_cast<IRConstantBool*>(operand)) {
        return std::to_string(c->GetValue());
    }
    else if (auto g = dynamic_cast<IRGlobalVariable*>(operand)) {
        return g->GetName();
    }
    else if (auto arg = dynamic_cast<IRArgument*>(operand))
    {
        // find argument index in current func (0-based)
        size_t foundIndex = 0;
        for (auto& a : this->m_currentFunc->GetArguments())
        {
            if (arg == a.get()) break;
            ++foundIndex;
        }

        if (TargetArchitecture::GetOS() == OS::Linux)
        {
            // first 6 args in registers RDI, RSI, RDX, RCX, R8, R9
            if (foundIndex <= 5)
            {
                return x86_64::linuxArgMapping[foundIndex];
            }
            else
            {
                // first stack-passed arg (7th argument, foundIndex==6) is at [rbp+16]
                // formula: offset = 8 * (foundIndex - 4)
                int stackOffset = 8 * static_cast<int>(foundIndex - 4);
                return "[rbp+" + std::to_string(stackOffset) + "]";
            }
        }
        else if (TargetArchitecture::GetOS() == OS::Windows)
        {
            // first 4 args in RCX,RDX,R8,R9
            if (foundIndex <= 3)
            {
                return x86_64::windowsArgMapping[foundIndex];
            }
            else
            {
                // On Windows the caller provides a 32-byte shadow area at entry.
                // After push rbp; mov rbp,rsp:
                // saved rbp: [rbp]
                // return addr: [rbp+8]
                // shadow space: [rbp+16 .. rbp+47]
                // first pushed arg (arg5, foundIndex==4) is at [rbp+48]
                // so offset = 48 + 8 * (foundIndex - 4)
                int stackOffset = 48 + 8 * static_cast<int>(foundIndex - 4);
                return "[rbp+" + std::to_string(stackOffset) + "]";
            }
        }
        else
        {
            return "";
        }
    }

    // otherwise it's a local / temp -> allocate stack slot (rbp - offset)
    return slotOfInstr(operand, out);
}

std::string X86_64Emitter::slotOfInstr(IRBaseValue* value, std::ostream& out)
{
    if (!value) return "";

    auto it = m_valueToStackOffset.find(value);
    if (it != m_valueToStackOffset.end())
    {
        return "[rbp-" + std::to_string(it->second) + "]"; // already assigned
    }

    out << "    sub rsp, 8\n"; // Grow frame.
    // allocate new stack slot
    int offset = this->m_nextTempStackOffset;
    this->m_nextTempStackOffset += 8;   // 8 bytes per slot
    m_valueToStackOffset[value] = offset;
    return "[rbp-" + std::to_string(offset) + "]";
}

void X86_64Emitter::LoadOperandAsFloat(IRBaseValue* operand, const std::string& xmmReg, std::ostream& out)
{
    // Float64 -> double precision (movsd)
    if (operand->GetType() == IRType::GetFloat64())
    {
        out << "    movsd " << xmmReg << ", qword " << GetOperandAsm(operand, out) << "\n";
    }
    // Float32 -> single precision (movss)
    else if (operand->GetType() == IRType::GetFloat32())
    {
        out << "    movss " << xmmReg << ", dword " << GetOperandAsm(operand, out) << "\n";
    }
    // Integer -> convert 64-bit int to double
    else
    {
        out << "    mov rax, " << GetOperandAsm(operand, out) << "\n"; // 64-bit integer
        out << "    cvtsi2sd " << xmmReg << ", rax\n";                 // convert to double
    }
}
