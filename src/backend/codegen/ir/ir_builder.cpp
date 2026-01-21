#include "includes/backend/codegen/ir/ir_builder.h"

IRBuilder::IRBuilder(const EntityLogger& entitylogger) :
    m_insertBlock(nullptr),
    m_entitylogger(entitylogger),
    m_tempCounter(0)
{}


void IRBuilder::SetInsertPoint(BasicBlock* block) noexcept
{
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
    return CreateBinaryOp(
        IROpcode::FAdd,
        lhs,
        rhs,
        lhs->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateFSub(IRBaseValue* lhs,
    IRBaseValue* rhs,
    const std::string& name)
{
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
    if (lhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateAnd: lhs is nullptr");
    }
    if (rhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateAnd: rhs is nullptr");
    }

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
    if (lhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateOr: lhs is nullptr");
    }
    if (rhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateOr: rhs is nullptr");
    }

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
    if (lhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateXor: lhs is nullptr");
    }
    if (rhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateXor: rhs is nullptr");
    }

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
    if (lhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateShl: lhs is nullptr");
    }
    if (rhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateShl: rhs is nullptr");
    }

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
    if (lhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateAShr: lhs is nullptr");
    }
    if (rhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateAShr: rhs is nullptr");
    }

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
    if (lhs == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateLShr:lhs is nullptr");
    }
    if (rhs == nullptr)
    {
        throw std::runtime_error("IRBuilder:CreateLShr: rhs is nullptr");
    }

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
    return CreateBinaryOp(
        IROpcode::Alloca,
        nullptr,
        nullptr,
        type,
        name
    );
}

IRInstruction* IRBuilder::CreateLoad(
    IRBaseValue* ptr,
    const std::string& name)
{
    if (this->m_insertBlock == nullptr)
    {
        throw std::runtime_error("IRBuilder: No insert block set");
    }

    if (ptr == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateLoad: ptr is null");
    }

    return CreateBinaryOp(
        IROpcode::Load,
        ptr,
        nullptr,
        ptr->GetType(),
        name
    );
}

IRInstruction* IRBuilder::CreateStore(IRBaseValue* value, IRBaseValue* ptr)
{
    // Store has no result (void), and no name
    auto inst = std::make_unique<IRInstruction>(
        "",  // No name
        IRType::GetVoid(),
        IROpcode::Store,
        std::vector<IRBaseValue*>{value, ptr},
        m_insertBlock
    );

    return Insert(std::move(inst));
}

void IRBuilder::CreateBr(BasicBlock* target)
{
    if (this->m_insertBlock == nullptr)
    {
        throw std::runtime_error("IRBuilder: No insert block set");
    }
    if (target == nullptr)
    {
        throw std::runtime_error("IRBuilder::CreateBr: target is null");
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        IRType::GetVoid(),
        IROpcode::Br,
        std::vector<IRBaseValue*>{},  // Target stored differently - see note
        m_insertBlock
    );
    
    // Update CFG
    m_insertBlock->AddSuccessor(target);
    target->AddPredecessor(m_insertBlock);
    
    m_insertBlock->AddInstruction(std::move(inst));
}

void IRBuilder::CreateCondBr(IRBaseValue* cond, BasicBlock* trueBB, BasicBlock* falseBB)
{
    if (this->m_insertBlock == nullptr)
    {
        throw std::runtime_error("IRBuilder: No insert block set");
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
}

void IRBuilder::CreateRet(IRBaseValue* value)
{
    if (this->m_insertBlock == nullptr)
    {
        throw std::runtime_error("IRBuilder: No insert block set");
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        value->GetType(),
        IROpcode::Ret,
        std::vector<IRBaseValue*>{value},
        m_insertBlock
    );
    
    m_insertBlock->AddInstruction(std::move(inst));
}

void IRBuilder::CreateRetVoid()
{
    if (this->m_insertBlock == nullptr)
    {
        throw std::runtime_error("IRBuilder: No insert block set");
    }

    auto inst = std::make_unique<IRInstruction>(
        "",
        IRType::GetVoid(),
        IROpcode::Ret,
        std::vector<IRBaseValue*>{},
        m_insertBlock
    );
    
    m_insertBlock->AddInstruction(std::move(inst));
}

IRInstruction* IRBuilder::CreateCall(
    IRFunction* func,
    const std::vector<IRBaseValue*>& args,
    const std::string& name)
{
    auto inst = std::make_unique<IRInstruction>(
        name,
        func->GetReturnType(),
        IROpcode::Call,
        args,
        this->m_insertBlock
    );

    inst->SetCalledFunction(func);

    return Insert(std::move(inst));
}

IRInstruction* IRBuilder::CreatePhi(
    std::shared_ptr<const IRType> type,
    const std::string& name)
{
    auto inst = std::make_unique<IRInstruction>(
        name,
        type,
        IROpcode::Phi,
        std::vector<IRBaseValue*>{}, // incoming values added later
        m_insertBlock
    );

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
    if (this->m_insertBlock == nullptr) 
    {
        throw std::runtime_error("IRBuilder: No insert block set");
    }

    IRInstruction* ptr = inst.get();
    this->m_insertBlock->AddInstruction(std::move(inst));
    return ptr;
}

IRInstruction* IRBuilder::InsertPhi(std::unique_ptr<IRInstruction> inst)
{
    if (this->m_insertBlock == nullptr)
    {
        throw std::runtime_error("IRBuilder: No insert block set");
    } 

    IRInstruction* ptr = inst.get();
    this->m_insertBlock->AddFirstInstruction(std::move(inst));
    return ptr;
}
