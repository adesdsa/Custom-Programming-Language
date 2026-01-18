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
    this->m_intConstants.push_back(
    std::make_unique<IRConstantInt>(value,  IRType::GetInt8())
    );
    return m_intConstants.back().get();
}
// same logic for ||
//                \/
IRConstantInt* IRBuilder::GetInt32(int64_t value);
IRConstantInt* IRBuilder::GetInt64(int64_t value);
IRConstantBool* IRBuilder::GetTrue();
IRConstantBool* IRBuilder::GetFalse();
IRConstantBool* IRBuilder::GetBool(bool value);
IRConstantFloat* IRBuilder::GetFloat(double value);
IRConstantFloat* IRBuilder::GetDouble(double value);

// ===== INTEGER ARITHMETIC =====
IRInstruction* IRBuilder::CreateAdd(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "")
{
    {
return CreateBinaryOp(IROpcode::Add, lhs, rhs, lhs->GetType(), name);
    }
}
// same logic ||
//            \/
IRInstruction* IRBuilder::CreateSub(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateMul(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateSDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateSRem(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

// ===== FLOAT ARITHMETIC =====
IRInstruction* IRBuilder::CreateFAdd(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateFSub(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateFMul(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateFDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

// ===== INTEGER COMPARISON =====
IRInstruction* IRBuilder::CreateICmpEQ(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "")
{
    {
return CreateBinaryOp(IROpcode::ICmpEq, lhs, rhs, IRType::GetInt1(), name);
    }
}
IRInstruction* IRBuilder::CreateICmpNE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateICmpSLT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateICmpSLE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateICmpSGT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateICmpSGE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

// ===== FLOAT COMPARISON =====
IRInstruction* IRBuilder::CreateFCmpEQ(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateFCmpLT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateFCmpLE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateFCmpGT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateFCmpGE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

// ===== BITWISE =====
IRInstruction* IRBuilder::CreateAnd(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateOr(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateXor(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateShl(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateAShr(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
IRInstruction* IRBuilder::CreateLShr(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

// ===== MEMORY =====
IRInstruction* IRBuilder::CreateAlloca(std::shared_ptr<const IRType> type, const std::string& name = "");
IRInstruction* IRBuilder::CreateLoad(IRBaseValue* ptr, const std::string& name = "");
IRInstruction* IRBuilder::CreateStore(IRBaseValue* value, IRBaseValue* ptr)
// Store has no result (void), and no name
auto inst = std::make_unique<IRInstruction>(
    "",  // No name
    IRType::GetVoid(),
    IROpcode::Store,
    std::vector<IRBaseValue*>{value, ptr},
    m_insertBlock
);

return Insert(std::move(inst));

// ===== CONTROL FLOW (Terminators) =====
void IRBuilder::CreateBr(BasicBlock* target)
{
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
void IRBuilder::CreateCondBr(IRBaseValue* cond, BasicBlock* trueBB, BasicBlock* falseBB);
void IRBuilder::CreateRet(IRBaseValue* value);
void IRBuilder::CreateRetVoid();

// ===== FUNCTION CALLS =====
IRInstruction* IRBuilder::CreateCall(IRFunction* func, const std::vector<IRBaseValue*>& args, const std::string& name = "");

/// @brief Generate next temporary name (t0, t1, t2...)
std::string IRBuilder::NextTemp();

/// @brief Helper to create binary operations
IRInstruction* IRBuilder::CreateBinaryOp(IROpcode opcode, 
                                IRBaseValue* lhs, 
                                IRBaseValue* rhs, 
                                std::shared_ptr<const IRType> resultType,
                                const std::string& name)
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

/// @brief Insert instruction into current block
IRInstruction* IRBuilder::Insert(std::unique_ptr<IRInstruction> inst);
{
    if (m_insertBlock == nullptr) 
    {
        throw std::runtime_error("IRBuilder: No insert block set");
    }

    IRInstruction* ptr = inst.get();
    m_insertBlock->AddInstruction(std::move(inst));
    return ptr;
}
