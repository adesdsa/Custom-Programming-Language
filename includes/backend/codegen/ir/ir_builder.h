
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/ir/ir_templates/ir_function.h"
#include "includes/backend/codegen/ir/ir_templates/ir_basic_block.h"
#include "includes/backend/codegen/ir/ir_templates/ir_instruction.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_int.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_float.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_bool.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_string.h"
#include "includes/shared/logger/entity_logger.h"
#include "includes/backend/codegen/ir/ir_common_types.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/shared/common/target_architecture.h"

/// @brief Helper class that simplifies IR construction.
///
/// IRBuilder provides convenient methods for creating IR instructions.
/// - CreateXxx() methods create and insert instructions in one call.
/// - GetXxx() methods create constants.
/// - Automatically generates temporary names (t0, t1, t2...) if not provided.
class IRBuilder 
{
public:
    IRBuilder(const EntityLogger& entitylogger);

    // ===== INSERTION POINT =====
    void SetInsertPoint(BasicBlock* block) noexcept;
    BasicBlock* GetInsertBlock() const noexcept;

    // ===== CONSTANTS =====
    IRConstantInt* GetInt8(int64_t value);
    IRConstantInt* GetInt32(int64_t value);
    IRConstantInt* GetInt64(int64_t value);
    IRConstantBool* GetTrue();
    IRConstantBool* GetFalse();
    IRConstantBool* GetBool(bool value);
    IRConstantFloat* GetFloat(double value);
    IRConstantFloat* GetDouble(double value);
    IRConstantString* GetString(const std::string& value);

    // ===== INTEGER ARITHMETIC =====
    IRInstruction* CreateAdd(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateSub(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateMul(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateSDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateSFloorDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateSRem(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateUDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateUFloorDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateURem(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

    // ===== FLOAT ARITHMETIC =====
    IRInstruction* CreateFAdd(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFSub(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFMul(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFFloorDiv(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFRem(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

    // ===== INTEGER COMPARISON =====
    IRInstruction* CreateICmpEQ(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpNE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpSLT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpSLE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpSGT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpSGE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpULT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpULE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpUGT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateICmpUGE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

    // ===== FLOAT COMPARISON =====
    IRInstruction* CreateFCmpEQ(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFCmpNE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFCmpLT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFCmpLE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFCmpGT(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateFCmpGE(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

    // ===== BITWISE =====
    IRInstruction* CreateAnd(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateOr(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateXor(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateShl(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateAShr(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");
    IRInstruction* CreateLShr(IRBaseValue* lhs, IRBaseValue* rhs, const std::string& name = "");

    // ===== MEMORY =====
    IRInstruction* CreateAlloca(std::shared_ptr<const IRType> type, const std::string& name = "");
    IRInstruction* CreateLoad(IRBaseValue* ptr, const std::string& name = "");
    IRInstruction* CreateStore(IRBaseValue* value, IRBaseValue* ptr);

    // ===== TYPE CONVERSION =====
    IRInstruction* CreateFPToSI(IRBaseValue* value, std::shared_ptr<const IRType> destType, const std::string& name = "");
    IRInstruction* CreateSIToFP(IRBaseValue* value, std::shared_ptr<const IRType> destType, const std::string& name = "");

    // ===== CONTROL FLOW (Terminators) =====
    void CreateBr(BasicBlock* target);
    void CreateCondBr(IRBaseValue* cond, BasicBlock* trueBB, BasicBlock* falseBB);
    void CreateRet(IRBaseValue* value);
    void CreateRetVoid();

    // ===== FUNCTION CALLS =====
    IRInstruction* CreateCall(IRFunction* func, const std::vector<IRBaseValue*>& args, const std::string& name = "");

    // ===== OTHERS =====
    IRInstruction* CreatePhi(std::shared_ptr<const IRType> type, const std::string& name);
    IRInstruction* CreateSelect(IRBaseValue* cond, IRBaseValue* trueVal, IRBaseValue* falseVal, const std::string& name = "");

    // ===== CONSTANTS =====
    /// @note After calling this function and moving all of the values using std::move call ClearConstants().
    std::vector<std::unique_ptr<IRBaseValue>> DumpConstants() noexcept;
    /// @note Must be called after all values are moved from m_constants.
    void ClearConstants() noexcept;

private:
    BasicBlock* m_insertBlock;   // Where new instructions go
    size_t m_tempCounter;        // For auto-generating names: t0, t1, t2...
    size_t m_strLabelCounter = 0; // For auto-generating string labels: __str0, __str1...
    std::vector<std::unique_ptr<IRBaseValue>> m_constants; // After moving the values call ClearConstants().
    
    const EntityLogger& m_entitylogger;
    ErrorManager& m_errorManager;

    /// @brief Generate next temporary name (t0, t1, t2...)
    std::string NextTemp();
    
    /// @brief Helper to create binary operations
    IRInstruction* CreateBinaryOp(
        IROpcode opcode, 
        IRBaseValue* lhs, 
        IRBaseValue* rhs, 
        std::shared_ptr<const IRType> resultType,
        const std::string& name
    );
    
    /// @brief Insert instruction into current block
    IRInstruction* Insert(std::unique_ptr<IRInstruction> inst);

    /// @brief Insert Phi instruction into current block, must be the first instruction
    IRInstruction* InsertPhi(std::unique_ptr<IRInstruction> inst);

    void StoreInternalError(std::string msg) const;
};
