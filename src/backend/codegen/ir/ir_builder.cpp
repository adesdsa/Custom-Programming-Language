#include "includes/backend/codegen/ir/ir_builder.h"

IRBuilder::IRBuilder(const EntityLogger& entitylogger) :
    m_insertBlock(nullptr),
    m_tempCounter(0),
    m_entitylogger(entitylogger),
    m_errorManager(ErrorManager::GetInstance())
{}


void IRBuilder::SetInsertPoint(BasicBlock* block) noexcept
{
    // No need to check that block!=nullptr because we might want to set that there isnt next one.
    this->m_insertBlock = block;
}

BasicBlock* IRBuilder::GetInsertBlock() const noexcept
{
    return this->m_insertBlock;
}

// ===== CONSTANTS =====
IRConstantInt* IRBuilder::GetInt8(int64_t value)
{
    this->m_constants.push_back(
    std::make_unique<IRConstantInt>(value,  IRType::GetInt8())
    );

    return static_cast<IRConstantInt*>(m_constants.back().get());
}

IRConstantInt* IRBuilder::GetInt32(int64_t value)
{
    this->m_constants.push_back(
    std::make_unique<IRConstantInt>(value,  IRType::GetInt32())
    );

    return static_cast<IRConstantInt*>(m_constants.back().get());
}

IRConstantInt* IRBuilder::GetInt64(int64_t value)
{
    this->m_constants.push_back(
    std::make_unique<IRConstantInt>(value,  IRType::GetInt64())
    );

    return static_cast<IRConstantInt*>(m_constants.back().get());
}

IRConstantBool* IRBuilder::GetTrue()
{
    this->m_constants.push_back(
    std::make_unique<IRConstantBool>(true)
    );

    return static_cast<IRConstantBool*>(m_constants.back().get());
}

IRConstantBool* IRBuilder::GetFalse()
{
    this->m_constants.push_back(
    std::make_unique<IRConstantBool>(false)
    );

    return static_cast<IRConstantBool*>(m_constants.back().get());
}

IRConstantBool* IRBuilder::GetBool(bool value)
{
    this->m_constants.push_back(
    std::make_unique<IRConstantBool>(value)
    );

    return static_cast<IRConstantBool*>(m_constants.back().get());
}

IRConstantFloat* IRBuilder::GetFloat(double value)
{
    this->m_constants.push_back(
    std::make_unique<IRConstantFloat>(value,  IRType::GetFloat32())
    );

    return static_cast<IRConstantFloat*>(m_constants.back().get());
}

IRConstantFloat* IRBuilder::GetDouble(double value)
{
    this->m_constants.push_back(
    std::make_unique<IRConstantFloat>(value,  IRType::GetFloat64())
    );

    return static_cast<IRConstantFloat*>(m_constants.back().get());
}

IRConstantString* IRBuilder::GetString(const std::string& value)
{
    this->m_constants.push_back(
        std::make_unique<IRConstantString>(value, IRType::GetInt64())
    );
    
    auto* strConst = static_cast<IRConstantString*>(m_constants.back().get());
    strConst->SetLabel("__str" + std::to_string(m_strLabelCounter++));

    return strConst;
}

// ===== INTEGER ARITHMETIC =====
IRInstruction* IRBuilder::CreateAdd(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateAdd: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateAdd: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new Add instruction");
    return CreateBinaryOp(
        IROpcode::Add,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateSub(
    IRBaseValue* lhs, 
    IRBaseValue* rhs, 
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateSub: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateSub: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new Sub instruction");
    return CreateBinaryOp(
        IROpcode::Sub,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateMul(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateMul: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateMul: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new Mul instruction");
    return CreateBinaryOp(
        IROpcode::Mul,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateSDiv(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateSDiv: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateSDiv: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new SDiv instruction");
    return CreateBinaryOp(
        IROpcode::SDiv,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateSFloorDiv(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFloorDiv: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFloorDiv: right operand is null");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;

    // q = lhs / rhs
    auto* q = CreateSDiv(lhs, rhs, instName + ".q");

    // r = lhs % rhs
    auto* r = CreateSRem(lhs, rhs, instName + ".r");

    // constants
    auto* zero = GetInt64(0);
    auto* one  = GetInt64(1);

    // r != 0
    auto* rNotZero = CreateICmpNE(r, zero, instName + ".r.nz");

    // lhs < 0
    auto* lhsNeg = CreateICmpSLT(lhs, zero, instName + ".lhs.neg");

    // rhs < 0
    auto* rhsNeg = CreateICmpSLT(rhs, zero, instName + ".rhs.neg");

    // signs differ
    auto* signsDiffer = CreateBinaryOp(
        IROpcode::Xor,
        lhsNeg,
        rhsNeg,
        lhsNeg->GetType(),
        instName + ".sign.diff"
    );

    // need adjust?
    auto* needAdjust = CreateBinaryOp(
        IROpcode::And,
        rNotZero,
        signsDiffer,
        rNotZero->GetType(),
        instName + ".need.adjust"
    );

    // q - 1
    auto* qMinusOne = CreateSub(q, one, instName + ".q.dec");

    m_entitylogger.Info("Created a new SFloorDiv instruction");
    // select(needAdjust, q-1, q)
    return CreateSelect(needAdjust, qMinusOne, q, instName);
}


IRInstruction* IRBuilder::CreateSRem(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateSRem: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateSRem: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new SRem instruction");
    return CreateBinaryOp(
        IROpcode::SRem,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateUDiv(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateUDiv: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateUDiv: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new UDiv instruction");
    return CreateBinaryOp(
        IROpcode::UDiv,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateUFloorDiv(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFloorDiv: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFloorDiv: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new UFloorDiv instruction");
    return CreateUDiv(lhs, rhs, name);
}

IRInstruction* IRBuilder::CreateURem(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateURem: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateURem: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new URem instruction");
    return CreateBinaryOp(
        IROpcode::URem,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateFAdd(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFAdd: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFAdd: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FAdd instruction");
    return CreateBinaryOp(
        IROpcode::FAdd,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateFSub(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFSub: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFSub: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FSub instruction");
    return CreateBinaryOp(
        IROpcode::FSub,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateFMul(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFMul: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFMul: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FMul instruction");
    return CreateBinaryOp(
        IROpcode::FMul,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateFDiv(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFDiv: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFDiv: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FDiv instruction");
    return CreateBinaryOp(
        IROpcode::FDiv,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateFFloorDiv(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFloorDiv: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFloorDiv: right operand is null");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;

    IRBaseValue* one = nullptr;
    IRBaseValue* zero = nullptr;
    if (TargetArchitecture::Is32Bit())
    {
        one = GetFloat(1.0);
        zero = GetFloat(0.0);
    }
    else
    {
        one = GetDouble(1.0);
        zero = GetDouble(0.0);
    }

    // div = lhs / rhs
    IRInstruction* div = CreateFDiv(lhs, rhs, NextTemp());

    // frac = FRem(div, 1.0)
    IRInstruction* frac = CreateFRem(div, one, NextTemp());

    // div < 0 ?
    IRInstruction* isNegative = CreateFCmpLT(div, zero, NextTemp());

    // frac != 0 ?
    IRInstruction* hasFraction = CreateFCmpNE(frac, zero, NextTemp());

    // needSubtract = isNegative && hasFraction
    IRInstruction* needSubtract = CreateAnd(isNegative, hasFraction, NextTemp());

    // div - 1.0
    IRInstruction* divMinusOne = CreateFSub(div, one, NextTemp());

    m_entitylogger.Info("Created a new FFloorDiv instruction");
    // select(needSubtract ? div-1 : div)
    return CreateSelect(needSubtract, divMinusOne, div, instName);
}

IRInstruction* IRBuilder::CreateFRem(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFRem: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFRem: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new URem instruction");
    return CreateBinaryOp(
        IROpcode::FRem,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpEQ(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{   
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpEQ: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpEQ: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpEQ instruction");
    return CreateBinaryOp(
        IROpcode::ICmpEq,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpNE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpNE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpNE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpNE instruction");
    return CreateBinaryOp(
        IROpcode::ICmpNe,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpSLT(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpSLT: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpSLT: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpSLT instruction");
    return CreateBinaryOp(
        IROpcode::ICmpSlt,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpSLE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpSLE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpSLE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpSLE instruction");
    return CreateBinaryOp(
        IROpcode::ICmpSle,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpSGT(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpSGT: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpSGT: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpSGT instruction");
    return CreateBinaryOp(
        IROpcode::ICmpSgt,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpSGE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpSGE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpSGE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpSGE instruction");
    return CreateBinaryOp(
        IROpcode::ICmpSge,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpULT(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpULT: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpULT: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpULT instruction");
    return CreateBinaryOp(
        IROpcode::ICmpUlt,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpULE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpULE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpULE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpULE instruction");
    return CreateBinaryOp(
        IROpcode::ICmpUle,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpUGT(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpUGT: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpUGT: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpUGT instruction");
    return CreateBinaryOp(
        IROpcode::ICmpUgt,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateICmpUGE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateICmpUGE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateICmpUGE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new ICmpUGE instruction");
    return CreateBinaryOp(
        IROpcode::ICmpUge,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateFCmpEQ(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFCmpEQ: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFCmpEQ: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FCmpEQ instruction");
    return CreateBinaryOp(
        IROpcode::FCmpEq,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateFCmpNE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFCmpNE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFCmpNE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FCmpNE instruction");
    return CreateBinaryOp(
        IROpcode::FCmpNe,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateFCmpLT(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFCmpLT: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFCmpLT: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FCmpLT instruction");
    return CreateBinaryOp(
        IROpcode::FCmpLt,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateFCmpLE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFCmpLE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFCmpLE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FCmpLE instruction");
    return CreateBinaryOp(
        IROpcode::FCmpLe,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateFCmpGT(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFCmpGT: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFCmpGT: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FCmpGT instruction");
    return CreateBinaryOp(
        IROpcode::FCmpGt,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateFCmpGE(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateFCmpGE: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateFCmpGE: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new FCmpGE instruction");
    return CreateBinaryOp(
        IROpcode::FCmpGe,
        lhs,
        rhs,
        IRType::GetInt1(),
        name
    );
}

IRInstruction* IRBuilder::CreateAnd(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateAnd: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateAnd: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new And instruction");
    return CreateBinaryOp(
        IROpcode::And,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateOr(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateOr: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateOr: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new Or instruction");
    return CreateBinaryOp(
        IROpcode::Or,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateXor(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateXor: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateXor: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new Xor instruction");
    return CreateBinaryOp(
        IROpcode::Xor,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateShl(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateShl: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateShl: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new Shl instruction");
    return CreateBinaryOp(
        IROpcode::Shl,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateAShr(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateAShr: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateAShr: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new AShr instruction");
    return CreateBinaryOp(
        IROpcode::AShr,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateLShr(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateLShr: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateLShr: right operand is null");
        return nullptr;
    }

    m_entitylogger.Info("Created a new LShr instruction");
    return CreateBinaryOp(
        IROpcode::LShr,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateAlloca(
    std::shared_ptr<const IRType> type,
    const std::string& name)
{
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }
    std::string instName = name.empty() ? NextTemp() : name;
    
    auto inst = std::make_unique<IRInstruction>(
        instName,
        type,
        IROpcode::Alloca,
        std::vector<IRBaseValue*>{},
        this->m_insertBlock
    );

    if (!inst.get())
    {
        StoreInternalError("Code generation: cannot insert a null instruction");
        return nullptr;
    }
    if (!this->m_insertBlock) 
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }

    IRInstruction* ptr = inst.get();
    this->m_insertBlock->AddFirstInstruction(std::move(inst));
    m_entitylogger.Info("Created a new Alloca instruction");
    return ptr;
}

IRInstruction* IRBuilder::CreateLoad(
    IRBaseValue* ptr,
    const std::string& name)
{
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }
    if (!ptr)
    {
        StoreInternalError("Code generation: cannot load from a null pointer");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;
    
    auto inst = std::make_unique<IRInstruction>(
        instName,
        ptr->GetType(),
        IROpcode::Load,
        std::vector<IRBaseValue*>{ptr},
        this->m_insertBlock
    );

    m_entitylogger.Info("Created a new Load instruction");
    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::CreateStore(IRBaseValue* value, IRBaseValue* ptr)
{
    if (!value)
    {
        StoreInternalError("Code generation: CreateStore: value is null");
        return nullptr;
    }
    if (!ptr)
    {
        StoreInternalError("Code generation: cannot store to a null pointer");
        return nullptr;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }
        
    // Store has no result (void), and no name
    auto inst = std::make_unique<IRInstruction>(
        "",  // No name
        IRType::GetVoid(),
        IROpcode::Store,
        std::vector<IRBaseValue*>{value, ptr},
        this->m_insertBlock
    );

    m_entitylogger.Info("Created a new Store instruction");
    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::CreateFPToSI(
    IRBaseValue* value,
    std::shared_ptr<const IRType> destType,
    const std::string& name)
{
    if (!value)
    {
        StoreInternalError("Code generation: CreateFPToSI: value is null");
        return nullptr;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;

    auto inst = std::make_unique<IRInstruction>(
        instName,
        destType,
        IROpcode::FPToSI,
        std::vector<IRBaseValue*>{value},
        this->m_insertBlock
    );

    m_entitylogger.Info("Created a new FPToSI instruction");
    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::CreateSIToFP(
    IRBaseValue* value,
    std::shared_ptr<const IRType> destType,
    const std::string& name)
{
    if (!value)
    {
        StoreInternalError("Code generation: CreateSIToFP: value is null");
        return nullptr;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;

    auto inst = std::make_unique<IRInstruction>(
        instName,
        destType,
        IROpcode::SIToFP,
        std::vector<IRBaseValue*>{value},
        this->m_insertBlock
    );

    m_entitylogger.Info("Created a new SIToFP instruction");
    return Insert(std::move(inst));
}

void IRBuilder::CreateBr(BasicBlock* target)
{
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return;
    }
    if (!target)
    {
        StoreInternalError("Code generation: CreateBr: target is null");
        return;
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        IRType::GetVoid(),
        IROpcode::Br,
        std::vector<IRBaseValue*>{},  // Target stored as successor to current block.
        this->m_insertBlock
    );
    
    // Update CFG
    this->m_insertBlock->AddSuccessor(target);
    target->AddPredecessor(m_insertBlock);
    
    this->m_insertBlock->AddInstruction(std::move(inst));
    m_entitylogger.Info("Handled a new Br instruction");
}

void IRBuilder::CreateCondBr(IRBaseValue* cond, BasicBlock* trueBB, BasicBlock* falseBB)
{
    if (!cond)
    {
        StoreInternalError("Code generation: CreateCondBr: cond is null");
        return;
    }
    if (!trueBB)
    {
        StoreInternalError("Code generation: CreateCondBr: trueBB is null");
        return;
    }
    if (!falseBB)
    {
        StoreInternalError("Code generation: CreateCondBr: falseBB is null");
        return;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return;
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        IRType::GetVoid(),
        IROpcode::CondBr,
        std::vector<IRBaseValue*>{cond},
        this->m_insertBlock
    );
    
    // Update CFG
    this->m_insertBlock->AddSuccessor(trueBB);
    trueBB->AddPredecessor(this->m_insertBlock);

    this->m_insertBlock->AddSuccessor(falseBB);
    falseBB->AddPredecessor(this->m_insertBlock);
    
    this->m_insertBlock->AddInstruction(std::move(inst));
    m_entitylogger.Info("Handled a new CondBr instruction");
}

void IRBuilder::CreateRet(IRBaseValue* value)
{
    if (!value)
    {
        StoreInternalError("Code generation: CreateRet: value is null");
        return;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return;
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        value->GetType(),
        IROpcode::Ret,
        std::vector<IRBaseValue*>{value},
        this->m_insertBlock
    );
    
    this->m_insertBlock->AddInstruction(std::move(inst));
    m_entitylogger.Info("Handled a new Ret instruction");
}

void IRBuilder::CreateRetVoid()
{
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return;
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        IRType::GetVoid(),
        IROpcode::Ret,
        std::vector<IRBaseValue*>{},
        this->m_insertBlock
    );
    
    this->m_insertBlock->AddInstruction(std::move(inst));
    m_entitylogger.Info("Handled a new RetVoid instruction");
}

IRInstruction* IRBuilder::CreateCall(
    IRFunction* func,
    const std::vector<IRBaseValue*>& args,
    const std::string& name)
{
    if (!func)
    {
        StoreInternalError("Code generation: CreateCall: func is null");
        return nullptr;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }

    auto inst = std::make_unique<IRInstruction>(
        name,
        func->GetReturnType(),
        IROpcode::Call,
        args,
        this->m_insertBlock
    );

    inst->SetCalledFunction(func);

    m_entitylogger.Info("Created a new Call instruction");
    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::CreatePhi(
    std::shared_ptr<const IRType> type,
    const std::string& name)
{
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }

    auto inst = std::make_unique<IRInstruction>(
        name,
        type,
        IROpcode::Phi,
        std::vector<IRBaseValue*>{}, // incoming values added later
        this->m_insertBlock
    );

    m_entitylogger.Info("Created a new Phi instruction");
    // Phi nodes must be first in the block
    return InsertPhi(std::move(inst));
}

IRInstruction* IRBuilder::CreateSelect(
    IRBaseValue* cond,
    IRBaseValue* trueVal,
    IRBaseValue* falseVal,
    const std::string& name)
{
    if (!cond || !trueVal || !falseVal)
    {
        StoreInternalError("Code generation: CreateSelect: nullptr operand");
        return nullptr;
    }

    if (!m_insertBlock)
    {
        StoreInternalError("Code generation: CreateSelect: No insert block set");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;

    auto inst = std::make_unique<IRInstruction>(
        instName,
        trueVal->GetType(),
        IROpcode::Select,
        std::vector<IRBaseValue*>{ cond, trueVal, falseVal },
        m_insertBlock
    );

    return Insert(std::move(inst));
}

std::vector<std::unique_ptr<IRBaseValue>> IRBuilder::DumpConstants() noexcept
{
    return std::move(this->m_constants);
}

void IRBuilder::ClearConstants() noexcept
{
    this->m_constants.clear();
}

std::string IRBuilder::NextTemp()
{
    std::string name = "t" + std::to_string(this->m_tempCounter);
    this->m_tempCounter++;
    m_entitylogger.Info("New temp is: " + name);
    return name;
}

IRInstruction* IRBuilder::CreateBinaryOp(
    IROpcode opcode, 
    IRBaseValue* lhs, 
    IRBaseValue* rhs, 
    std::shared_ptr<const IRType> resultType,
    const std::string& name
)
{
    if (!lhs)
    {
        StoreInternalError("Code generation: CreateBinaryOp: left operand is null");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("Code generation: CreateBinaryOp: right operand is null");
        return nullptr;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;
    
    auto inst = std::make_unique<IRInstruction>(
        instName,
        resultType,
        opcode,
        std::vector<IRBaseValue*>{lhs, rhs},
        this->m_insertBlock
    );

    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::Insert(std::unique_ptr<IRInstruction> inst)
{
    if (!inst.get())
    {
        StoreInternalError("Code generation: cannot insert a null instruction");
        return nullptr;
    }
    if (!this->m_insertBlock) 
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    }

    IRInstruction* ptr = inst.get();
    this->m_insertBlock->AddInstruction(std::move(inst));
    return ptr;
}

IRInstruction* IRBuilder::InsertPhi(std::unique_ptr<IRInstruction> inst)
{
    if (!inst.get())
    {
        StoreInternalError("Code generation: cannot insert a null instruction");
        return nullptr;
    }
    if (!this->m_insertBlock)
    {
        StoreInternalError("IRBuilder: No insert block set");
        return nullptr;
    } 

    IRInstruction* ptr = inst.get();
    this->m_insertBlock->AddFirstInstruction(std::move(inst));
    return ptr;
}

void IRBuilder::StoreInternalError(std::string msg) const
{
    m_errorManager.StoreError(
        CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            msg,
            0,
            0,
            ""
        )
    );
}
