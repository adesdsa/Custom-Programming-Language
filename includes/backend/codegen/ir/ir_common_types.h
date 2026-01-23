#pragma once

enum class IROpcode
{
    // Arithmetic
    Add,        // %r = add i32 %a, %b
    Sub,        // %r = sub i32 %a, %b
    Mul,        // %r = mul i32 %a, %b
    SDiv,       // %r = sdiv i32 %a, %b  (signed division)
    UDiv,       // %r = udiv i32 %a, %b  (unsigned division)
    SRem,       // %r = srem i32 %a, %b  (signed remainder/modulo)
    URem,       // %r = urem i32 %a, %b  (unsigned remainder)

    // Floating point arithmetic
    FAdd,       // %r = fadd float %a, %b
    FSub,       // %r = fsub float %a, %b
    FMul,       // %r = fmul float %a, %b
    FDiv,       // %r = fdiv float %a, %b

    // Bitwise
    And,        // %r = and i32 %a, %b
    Or,         // %r = or i32 %a, %b
    Xor,        // %r = xor i32 %a, %b
    Shl,        // %r = shl i32 %a, %b   (shift left)
    LShr,       // %r = lshr i32 %a, %b  (logical shift right)
    AShr,       // %r = ashr i32 %a, %b  (arithmetic shift right)

    // Comparison
    ICmpEq,     // %r = icmp eq i32 %a, %b   (==)
    ICmpNe,     // %r = icmp ne i32 %a, %b   (!=)
    ICmpSlt,    // %r = icmp slt i32 %a, %b  (signed <)
    ICmpSle,    // %r = icmp sle i32 %a, %b  (signed <=)
    ICmpSgt,    // %r = icmp sgt i32 %a, %b  (signed >)
    ICmpSge,    // %r = icmp sge i32 %a, %b  (signed >=)

    ICmpUlt,    // %r = icmp ult i32 %a, %b  (unsigned <)
    ICmpUle,    // %r = icmp ule i32 %a, %b  (unsigned <=)
    ICmpUgt,    // %r = icmp ugt i32 %a, %b  (unsigned >)
    ICmpUge,     // %r = icmp uge i32 %a, %b  (unsigned >=)

    FCmpEq,     // %r = fcmp eq float %a, %b   (==)
    FCmpNe,     // %r = fcmp ne float %a, %b   (!=)
    FCmpLt,     // %r = fcmp lt float %a, %b  (<)
    FCmpLe,     // %r = fcmp le float %a, %b  (<=)
    FCmpGt,     // %r = fcmp gt float %a, %b  (>)
    FCmpGe,     // %r = fcmp ge float %a, %b  (>=)

    // Memory
    Alloca,     // %ptr = alloca i32         (allocate stack space)
    Load,       // %val = load i32, i32* %ptr (read from memory)
    Store,      // store i32 %val, i32* %ptr  (write to memory)

    // Control flow
    Br,         // br label %target          (unconditional jump)
    CondBr,     // br i1 %cond, label %t, label %f  (conditional jump)
    Ret,        // ret i32 %val              (return from function)
    Call,       // %r = call i32 @func(...)  (function call)

    // SSA
    Phi         // %r = phi i32 [%a, %bb1], [%b, %bb2]
};
