#include "includes/backend/codegen/ir/ir_builder.h"

IRBuilder::IRBuilder(const EntityLogger& entitylogger) :
    m_insertBlock(nullptr),
    m_entitylogger(entitylogger),
    m_tempCounter(0),
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

// ===== INTEGER ARITHMETIC =====
IRInstruction* IRBuilder::CreateAdd(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("IRBuilder::CreateAdd: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateAdd: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateSub: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateSub: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateMul: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateMul: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateSDiv: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateSDiv: rhs is nullptr");
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

IRInstruction* IRBuilder::CreateSRem(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("IRBuilder::CreateSRem: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateSRem: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateUDiv: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateUDiv: rhs is nullptr");
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

IRInstruction* IRBuilder::CreateURem(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
    if (!lhs)
    {
        StoreInternalError("IRBuilder::CreateURem: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateURem: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFAdd: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFAdd: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFSub: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFSub: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFMul: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFMul: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFDiv: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFDiv: rhs is nullptr");
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

IRInstruction* IRBuilder::CreateICmpEQ(
    IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{   
    if (!lhs)
    {
        StoreInternalError("IRBuilder::CreateICmpEQ: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpEQ: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpNE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpNE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpSLT: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpSLT: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpSLE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpSLE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpSGT: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpSGT: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpSGE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpSGE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpULT: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpULT: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpULE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpULE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpUGT: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpUGT: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateICmpUGE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateICmpUGE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFCmpEQ: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFCmpEQ: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFCmpNE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFCmpNE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFCmpLT: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFCmpLT: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFCmpLE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFCmpLE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFCmpGT: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFCmpGT: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateFCmpGE: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateFCmpGE: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateAnd: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateAnd: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateOr: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateOr: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateXor: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateXor: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateShl: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateShl: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateAShr: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateAShr: rhs is nullptr");
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
        StoreInternalError("IRBuilder::CreateLShr: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateLShr: rhs is nullptr");
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
        name,
        type,
        IROpcode::Alloca,
        std::vector<IRBaseValue*>{},
        m_insertBlock
    );

    m_entitylogger.Info("Created a new Alloca instruction");
    return Insert(std::move(inst));
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
        StoreInternalError("IRBuilder::CreateLoad: ptr is null");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;
    
    auto inst = std::make_unique<IRInstruction>(
        name,
        ptr->GetType(),
        IROpcode::Load,
        std::vector<IRBaseValue*>{ptr},
        m_insertBlock
    );

    m_entitylogger.Info("Created a new Load instruction");
    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::CreateStore(IRBaseValue* value, IRBaseValue* ptr)
{
    if (!value)
    {
        StoreInternalError("IRBuilder::CreateStore: value is null");
        return nullptr;
    }
    if (!ptr)
    {
        StoreInternalError("IRBuilder::CreateStore: ptr is null");
    }
        
    // Store has no result (void), and no name
    auto inst = std::make_unique<IRInstruction>(
        "",  // No name
        IRType::GetVoid(),
        IROpcode::Store,
        std::vector<IRBaseValue*>{value, ptr},
        m_insertBlock
    );

    m_entitylogger.Info("Created a new Store instruction");
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
        StoreInternalError("IRBuilder::CreateBr: target is null");
        return;
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        IRType::GetVoid(),
        IROpcode::Br,
        std::vector<IRBaseValue*>{},  // Target stored as successor to current block.
        m_insertBlock
    );
    
    // Update CFG
    m_insertBlock->AddSuccessor(target);
    target->AddPredecessor(m_insertBlock);
    
    m_insertBlock->AddInstruction(std::move(inst));
    m_entitylogger.Info("Handled a new Br instruction");
}

void IRBuilder::CreateCondBr(IRBaseValue* cond, BasicBlock* trueBB, BasicBlock* falseBB)
{
    if (!cond)
    {
        StoreInternalError("IRBuilder::CreateCondBr: cond is null");
        return nullptr;
    }
    if (!trueBB)
    {
        StoreInternalError("IRBuilder::CreateCondBr: trueBB is null");
    }
    if (!falseBB)
    {
        StoreInternalError("IRBuilder::CreateCondBr: falseBB is null");
        return nullptr;
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
        m_insertBlock
    );
    
    // Update CFG
    m_insertBlock->AddSuccessor(trueBB);
    trueBB->AddPredecessor(m_insertBlock);

    m_insertBlock->AddSuccessor(falseBB);
    falseBB->AddPredecessor(m_insertBlock);
    
    m_insertBlock->AddInstruction(std::move(inst));
    m_entitylogger.Info("Handled a new CondBr instruction");
}

void IRBuilder::CreateRet(IRBaseValue* value)
{
    if (!value)
    {
        StoreInternalError("IRBuilder::CreateRet: value is null");
        return nullptr;
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
        m_insertBlock
    );
    
    m_insertBlock->AddInstruction(std::move(inst));
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
        m_insertBlock
    );
    
    m_insertBlock->AddInstruction(std::move(inst));
    m_entitylogger.Info("Handled a new RetVoid instruction");
}

IRInstruction* IRBuilder::CreateCall(
    IRFunction* func,
    const std::vector<IRBaseValue*>& args,
    const std::string& name)
{
    if (!func)
    {
        StoreInternalError("IRBuilder::CreateCall: func is null");
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
        return;
    }

    auto inst = std::make_unique<IRInstruction>(
        name,
        type,
        IROpcode::Phi,
        std::vector<IRBaseValue*>{}, // incoming values added later
        m_insertBlock
    );

    m_entitylogger.Info("Created a new Phi instruction");
    // Phi nodes must be first in the block
    return InsertPhi(std::move(inst));
}

const std::vector<std::unique_ptr<IRBaseValue>>& IRBuilder::DumpConstants() const noexcept
{
    return this->m_constants;
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
        StoreInternalError("IRBuilder::CreateBinaryOp: lhs is nullptr");
        return nullptr;
    }
    if (!rhs)
    {
        StoreInternalError("IRBuilder::CreateBinaryOp: rhs is nullptr");
        return nullptr;
    }

    std::string instName = name.empty() ? NextTemp() : name;
    
    auto inst = std::make_unique<IRInstruction>(
        instName,
        resultType,
        opcode,
        std::vector<IRBaseValue*>{lhs, rhs},
        m_insertBlock
    );

    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::Insert(std::unique_ptr<IRInstruction> inst)
{
    if (!inst.get())
    {
        StoreInternalError("IRBuilder::Insert: inst is nullptr");
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
        StoreInternalError("IRBuilder::Insert: inst is nullptr");
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
