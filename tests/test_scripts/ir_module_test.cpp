#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/frontend/parser/parser_logic.h"
#include "includes/frontend/semantic_analysis/semantic_analysis_logic.h"
#include "includes/backend/codegen/ir/ir_generator.h"
#include "includes/backend/codegen/ir/ir_builder.h"
#include "includes/backend/codegen/ir/optimizations/optimization_manager.h"
#include "includes/backend/codegen/ir/optimizations/constant_folding_pass.h"
#include "includes/backend/codegen/ir/optimizations/dce_pass.h"
#include "includes/backend/codegen/target_architecture.h"
#include "includes/shared/utils/tests_utils.h"
#include "includes/shared/utils/system_utils.h"

// get expected int type based on architecture
IRTypeKind GetExpectedIntType() 
{
    return TargetArchitecture::Is32Bit() ? IRTypeKind::Int32 : IRTypeKind::Int64;
}

// TEST: Happy Flow - Single function
TEST(IRTest, HappyFlowSmallCodeExample) 
{
    // Setup: Parse -> Semantic Analysis -> IR Generation
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/small_code_example_file.txt"));
    const AST& ast = parser.MainFlow();
    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);
    const DecoratedAST& decoratedAST = semanticAnalysisComponent.MainFlow();
    
    IRGenerator irGenerator(decoratedAST);

    EntityLogger testLogger("GENERAL");
    testLogger.Info("Running IR Generator MainFlow");
    
    const IRModule& irModule = irGenerator.MainFlow();

    /////////////////////////////////////////////////////////////////
    // Module Structure Checks                                      |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating IR Module structure");

    // Module should contain exactly 1 function (Add)
    EXPECT_EQ(irModule.GetNumFunctions(), 1);
    EXPECT_EQ(irModule.GetNumVariables(), 0);  // No global variables

    /////////////////////////////////////////////////////////////////
    // Function: Add                                                |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating Add function");

    ASSERT_TRUE(irModule.IsFuncExistByName("Add"));
    IRFunction* addFunc = irModule.GetFunction("Add");
    ASSERT_NE(addFunc, nullptr);

    // Check function name
    EXPECT_EQ(addFunc->GetName(), "Add");

    // Check return type: int -> Int32/Int64 based on architecture
    std::shared_ptr<const IRType> returnType = addFunc->GetReturnType();
    ASSERT_NE(returnType, nullptr);
    EXPECT_EQ(returnType->GetKind(), GetExpectedIntType());

    // Check arguments: (a: int, b: int) -> 2 arguments, both Int32/Int64
    EXPECT_EQ(addFunc->GetNumArguments(), 2);

    IRArgument* argA = addFunc->GetArgument("a");
    ASSERT_NE(argA, nullptr);
    EXPECT_EQ(argA->GetName(), "a");
    EXPECT_EQ(argA->GetIndex(), 0);
    EXPECT_EQ(argA->GetType()->GetKind(), GetExpectedIntType());

    IRArgument* argB = addFunc->GetArgument("b");
    ASSERT_NE(argB, nullptr);
    EXPECT_EQ(argB->GetName(), "b");
    EXPECT_EQ(argB->GetIndex(), 1);
    EXPECT_EQ(argB->GetType()->GetKind(), GetExpectedIntType());

    /////////////////////////////////////////////////////////////////
    // Basic Block Checks                                           |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating basic blocks");

    // Should have at least entry block
    EXPECT_GE(addFunc->GetNumBasicBlocks(), 1);

    BasicBlock* entryBlock = addFunc->GetEntryBlock();
    ASSERT_NE(entryBlock, nullptr);

    // Entry block should have a terminator (ret instruction)
    EXPECT_TRUE(entryBlock->HasTerminator());
    
    IRInstruction* terminator = entryBlock->GetTerminator();
    ASSERT_NE(terminator, nullptr);
    EXPECT_EQ(terminator->GetOpcode(), IROpcode::Ret);

    /////////////////////////////////////////////////////////////////
    // Arithmetic Operations Check (return 3 + 2)                   |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating arithmetic operations");

    // Find an Add instruction in the function (3 + 2)
    bool foundAdd = false;
    for (const auto& block : addFunc->GetBasicBlocks())
    {
        for (const auto& instruction : block->GetInstructions())
        {
            if (instruction->GetOpcode() == IROpcode::Add)
            {
                foundAdd = true;
                // Add instruction should have 2 operands
                EXPECT_EQ(instruction->GetNumOperands(), 2);
                break;
            }
        }
        if (foundAdd) break;
    }

    EXPECT_TRUE(foundAdd) << "Expected to find an Add instruction for '3 + 2'";

    testLogger.Info("HappyFlowSmallCodeExample PASSED");
}


// TEST: Happy Flow - main function + other function with complex logic - Complete validation
TEST(IRTest, HappyFlowFullProgram) 
{
    // Setup: Parse -> Semantic Analysis -> IR Generation
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/full_program_example_file.txt"));
    const AST& ast = parser.MainFlow();
    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);
    const DecoratedAST& decoratedAST = semanticAnalysisComponent.MainFlow();
    
    IRGenerator irGenerator(decoratedAST);

    EntityLogger testLogger("GENERAL");
    testLogger.Info("Running IR Generator MainFlow on full program");
    
    const IRModule& irModule = irGenerator.MainFlow();

    /////////////////////////////////////////////////////////////////
    // Module Structure Checks                                      |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating IR Module structure");

    // Module should contain 2 functions (main + displayAllPrimeNumbersInRange)
    EXPECT_EQ(irModule.GetNumFunctions(), 2);

    /////////////////////////////////////////////////////////////////
    // Function: main                                               |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating main function");

    ASSERT_TRUE(irModule.IsFuncExistByName("main"));
    IRFunction* mainFunc = irModule.GetFunction("main");
    ASSERT_NE(mainFunc, nullptr);

    // Check function name
    EXPECT_EQ(mainFunc->GetName(), "main");

    // Check return type: int -> Int32/Int64 based on architecture
    std::shared_ptr<const IRType> mainReturnType = mainFunc->GetReturnType();
    ASSERT_NE(mainReturnType, nullptr);
    EXPECT_EQ(mainReturnType->GetKind(), GetExpectedIntType());

    // Check arguments: (void) -> 0 arguments
    EXPECT_EQ(mainFunc->GetNumArguments(), 0);

    // Should have entry block with terminator
    EXPECT_GE(mainFunc->GetNumBasicBlocks(), 1);
    BasicBlock* mainEntryBlock = mainFunc->GetEntryBlock();
    ASSERT_NE(mainEntryBlock, nullptr);
    EXPECT_TRUE(mainEntryBlock->HasTerminator());

    // Verify main has local variables (low, high) via alloca instructions
    IRInstruction* lowAlloca = mainFunc->FindAllocatedVar("low");
    ASSERT_NE(lowAlloca, nullptr);
    EXPECT_EQ(lowAlloca->GetOpcode(), IROpcode::Alloca);

    IRInstruction* highAlloca = mainFunc->FindAllocatedVar("high");
    ASSERT_NE(highAlloca, nullptr);
    EXPECT_EQ(highAlloca->GetOpcode(), IROpcode::Alloca);

    /////////////////////////////////////////////////////////////////
    // Function Call Check (displayAllPrimeNumbersInRange(low, high))|
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating function call instruction");

    // Find a Call instruction in main's basic blocks
    bool foundCall = false;
    for (const auto& block : mainFunc->GetBasicBlocks())
    {
        for (const auto& instruction : block->GetInstructions())
        {
            if (instruction->IsFuncCall())
            {
                foundCall = true;
                EXPECT_EQ(instruction->GetOpcode(), IROpcode::Call);
                
                // Verify it calls displayAllPrimeNumbersInRange
                IRFunction* calledFunc = instruction->GetCalledFunction();
                ASSERT_NE(calledFunc, nullptr);
                EXPECT_EQ(calledFunc->GetName(), "displayAllPrimeNumbersInRange");
                
                // Should have 2 operands (low, high arguments)
                EXPECT_EQ(instruction->GetNumOperands(), 2);
                break;
            }
        }
        if (foundCall) break;
    }

    EXPECT_TRUE(foundCall) << "Expected to find a function call instruction in main";

    /////////////////////////////////////////////////////////////////
    // Function: displayAllPrimeNumbersInRange                      |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating displayAllPrimeNumbersInRange function");

    ASSERT_TRUE(irModule.IsFuncExistByName("displayAllPrimeNumbersInRange"));
    IRFunction* displayFunc = irModule.GetFunction("displayAllPrimeNumbersInRange");
    ASSERT_NE(displayFunc, nullptr);

    // Check function name
    EXPECT_EQ(displayFunc->GetName(), "displayAllPrimeNumbersInRange");

    // Check return type: void -> Void
    std::shared_ptr<const IRType> displayReturnType = displayFunc->GetReturnType();
    ASSERT_NE(displayReturnType, nullptr);
    EXPECT_EQ(displayReturnType->GetKind(), IRTypeKind::Void);

    // Check arguments: (low: int, high: int) -> 2 arguments, both Int32/Int64
    EXPECT_EQ(displayFunc->GetNumArguments(), 2);

    IRArgument* lowArg = displayFunc->GetArgument("low");
    ASSERT_NE(lowArg, nullptr);
    EXPECT_EQ(lowArg->GetName(), "low");
    EXPECT_EQ(lowArg->GetIndex(), 0);
    EXPECT_EQ(lowArg->GetType()->GetKind(), GetExpectedIntType());

    IRArgument* highArg = displayFunc->GetArgument("high");
    ASSERT_NE(highArg, nullptr);
    EXPECT_EQ(highArg->GetName(), "high");
    EXPECT_EQ(highArg->GetIndex(), 1);
    EXPECT_EQ(highArg->GetType()->GetKind(), GetExpectedIntType());

    // Entry block should exist
    BasicBlock* displayEntryBlock = displayFunc->GetEntryBlock();
    ASSERT_NE(displayEntryBlock, nullptr);

    /////////////////////////////////////////////////////////////////
    // Control Flow Structure Checks (while, for, if)               |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating control flow structure");

    // Function has while, for, if - should have multiple blocks
    size_t numBlocks = displayFunc->GetNumBasicBlocks();
    EXPECT_GT(numBlocks, 3) << "Complex control flow should generate multiple basic blocks";

    // Check for conditional branch instructions (from if/while/for)
    bool foundCondBr = false;
    bool foundBr = false;
    
    for (const auto& block : displayFunc->GetBasicBlocks())
    {
        IRInstruction* blockTerminator = block->GetTerminator();
        if (blockTerminator != nullptr)
        {
            if (blockTerminator->GetOpcode() == IROpcode::CondBr)
            {
                foundCondBr = true;
            }
            else if (blockTerminator->GetOpcode() == IROpcode::Br)
            {
                foundBr = true;
            }
        }
    }

    EXPECT_TRUE(foundCondBr) << "Expected conditional branches from if/while/for statements";
    EXPECT_TRUE(foundBr) << "Expected unconditional branches for control flow";

    /////////////////////////////////////////////////////////////////
    // Comparison Operators Check (low < high, low == 0, etc.)      |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating comparison operations");

    bool foundComparison = false;
    for (const auto& block : displayFunc->GetBasicBlocks())
    {
        for (const auto& instruction : block->GetInstructions())
        {
            IROpcode opcode = instruction->GetOpcode();
            if (opcode == IROpcode::ICmpEq || opcode == IROpcode::ICmpNe ||
                opcode == IROpcode::ICmpSlt || opcode == IROpcode::ICmpSle ||
                opcode == IROpcode::ICmpSgt || opcode == IROpcode::ICmpSge)
            {
                foundComparison = true;
                EXPECT_EQ(instruction->GetNumOperands(), 2);
                break;
            }
        }
        if (foundComparison) break;
    }

    EXPECT_TRUE(foundComparison) << "Expected comparison instructions for conditions";

    /////////////////////////////////////////////////////////////////
    // Arithmetic Operators Check (i + 1, low % i, etc.)            |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating arithmetic operations");

    bool foundArithmetic = false;
    bool foundModulo = false;
    for (const auto& block : displayFunc->GetBasicBlocks())
    {
        for (const auto& instruction : block->GetInstructions())
        {
            IROpcode opcode = instruction->GetOpcode();
            if (opcode == IROpcode::Add || opcode == IROpcode::Sub ||
                opcode == IROpcode::Mul || opcode == IROpcode::SDiv)
            {
                foundArithmetic = true;
            }
            if (opcode == IROpcode::SRem || opcode == IROpcode::URem)
            {
                foundModulo = true;
            }
        }
    }

    EXPECT_TRUE(foundArithmetic) << "Expected arithmetic instructions (i + 1)";
    EXPECT_TRUE(foundModulo) << "Expected modulo instruction (low % i)";

    testLogger.Info("HappyFlowFullProgram PASSED");
}

/////////////////////////////////////////////////////////////////
// OPTIMIZATION TESTS                                           |
/////////////////////////////////////////////////////////////////

// Helper to get integer type based on architecture
std::shared_ptr<const IRType> GetArchIntType()
{
    return TargetArchitecture::Is32Bit() ? IRType::GetInt32() : IRType::GetInt64();
}

// TEST: Constant Folding - Happy Flow - Constant expressions should be folded
TEST(OptimizationTest, ConstantFoldingHappyFlow) 
{
    EntityLogger testLogger("OPTIMIZATION_TEST");
    testLogger.Info("Testing Constant Folding - Happy Flow");

    // Create an IRModule with a function that has foldable constant expressions
    IRModule module("ConstantFoldingTest");
    
    auto intType = GetArchIntType();
    
    // Create function: int testFunc() { return 3 + 5; }
    std::vector<std::shared_ptr<const IRType>> noParams;
    IRFunction* testFunc = module.AddFunction(
        "testFunc",
        IRType::GetFunction(intType, noParams)
    );
    ASSERT_NE(testFunc, nullptr);

    BasicBlock* entryBlock = testFunc->AddBasicBlock("entry");
    ASSERT_NE(entryBlock, nullptr);

    IRBuilder builder(testLogger);
    builder.SetInsertPoint(entryBlock);

    // Create: t0 = add int 3, 5  (this should be folded to 8)
    IRConstantInt* const3 = TargetArchitecture::Is32Bit() ? builder.GetInt32(3) : builder.GetInt64(3);
    IRConstantInt* const5 = TargetArchitecture::Is32Bit() ? builder.GetInt32(5) : builder.GetInt64(5);
    IRInstruction* addInst = builder.CreateAdd(const3, const5, "t0");
    ASSERT_NE(addInst, nullptr);

    // Create: ret i32 t0
    builder.CreateRet(addInst);

    // Move constants to module
    auto constants = builder.DumpConstants();
    for (auto& c : constants)
    {
        module.AddConstant(std::move(c));
    }
    builder.ClearConstants();

    // Verify before optimization: should have Add instruction
    size_t instructionCountBefore = entryBlock->GetInstructions().size();
    EXPECT_EQ(instructionCountBefore, 2); // add + ret

    bool foundAddBefore = false;
    for (const auto& inst : entryBlock->GetInstructions())
    {
        if (inst->GetOpcode() == IROpcode::Add)
        {
            foundAddBefore = true;
            break;
        }
    }
    EXPECT_TRUE(foundAddBefore) << "Add instruction should exist before optimization";

    // Run Constant Folding pass
    ConstantFoldingPass constantFoldingPass(testLogger);
    bool changed = constantFoldingPass.Run(module);

    // Verify: optimization should have made changes
    EXPECT_TRUE(changed) << "Constant folding should report changes made";

    // Verify: Add instruction should be removed (folded to constant 8)
    bool foundAddAfter = false;
    for (const auto& inst : entryBlock->GetInstructions())
    {
        if (inst->GetOpcode() == IROpcode::Add)
        {
            foundAddAfter = true;
            break;
        }
    }
    EXPECT_FALSE(foundAddAfter) << "Add instruction should be folded away";

    // Verify: ret instruction should now use constant 8 directly
    IRInstruction* retInst = entryBlock->GetTerminator();
    ASSERT_NE(retInst, nullptr);
    EXPECT_EQ(retInst->GetOpcode(), IROpcode::Ret);
    
    if (retInst->GetNumOperands() > 0)
    {
        IRBaseValue* retOperand = retInst->GetOperand(0);
        auto* constInt = dynamic_cast<IRConstantInt*>(retOperand);
        if (constInt)
        {
            EXPECT_EQ(constInt->GetValue(), 8) << "Return value should be folded constant 8";
        }
    }

    testLogger.Info("ConstantFoldingHappyFlow PASSED");
}

// TEST: Constant Folding - Negative Flow - No constants to fold
TEST(OptimizationTest, ConstantFoldingNegativeFlow) 
{
    EntityLogger testLogger("OPTIMIZATION_TEST");
    testLogger.Info("Testing Constant Folding - Negative Flow");

    // Create an IRModule with a function that has NO foldable expressions
    IRModule module("ConstantFoldingNegTest");
    
    auto intType = GetArchIntType();
    
    // Create function: int add(int a, int b) { return a + b; }
    std::vector<std::shared_ptr<const IRType>> params = { intType, intType };
    IRFunction* addFunc = module.AddFunction(
        "add",
        IRType::GetFunction(intType, params)
    );
    ASSERT_NE(addFunc, nullptr);

    IRArgument* argA = addFunc->AddArgument("a", intType);
    IRArgument* argB = addFunc->AddArgument("b", intType);
    ASSERT_NE(argA, nullptr);
    ASSERT_NE(argB, nullptr);

    BasicBlock* entryBlock = addFunc->AddBasicBlock("entry");
    ASSERT_NE(entryBlock, nullptr);

    IRBuilder builder(testLogger);
    builder.SetInsertPoint(entryBlock);

    // Create: t0 = add i32 %a, %b (cannot be folded - variables not constants)
    IRInstruction* addInst = builder.CreateAdd(argA, argB, "t0");
    ASSERT_NE(addInst, nullptr);

    // Create: ret i32 t0
    builder.CreateRet(addInst);

    // Verify before optimization
    size_t instructionCountBefore = entryBlock->GetInstructions().size();
    EXPECT_EQ(instructionCountBefore, 2); // add + ret

    // Run Constant Folding pass
    ConstantFoldingPass constantFoldingPass(testLogger);
    bool changed = constantFoldingPass.Run(module);

    // Verify: optimization should NOT have made changes (nothing to fold)
    EXPECT_FALSE(changed) << "Constant folding should not change code with only variables";

    // Verify: instruction count unchanged
    size_t instructionCountAfter = entryBlock->GetInstructions().size();
    EXPECT_EQ(instructionCountAfter, instructionCountBefore);

    // Verify: Add instruction still exists
    bool foundAdd = false;
    for (const auto& inst : entryBlock->GetInstructions())
    {
        if (inst->GetOpcode() == IROpcode::Add)
        {
            foundAdd = true;
            // Operands should still be the arguments
            EXPECT_EQ(inst->GetOperand(0), argA);
            EXPECT_EQ(inst->GetOperand(1), argB);
            break;
        }
    }
    EXPECT_TRUE(foundAdd) << "Add instruction should remain unchanged";

    testLogger.Info("ConstantFoldingNegativeFlow PASSED");
}

// TEST: DCE - Happy Flow - Dead code should be removed
TEST(OptimizationTest, DCEHappyFlow) 
{
    EntityLogger testLogger("OPTIMIZATION_TEST");
    testLogger.Info("Testing DCE - Happy Flow");

    // Create an IRModule with dead code
    IRModule module("DCETest");
    
    auto intType = GetArchIntType();
    bool is32Bit = TargetArchitecture::Is32Bit();
    
    // Create function: int testFunc() { int unused = 3 + 5; return 42; }
    std::vector<std::shared_ptr<const IRType>> noParams;
    IRFunction* testFunc = module.AddFunction(
        "testFunc",
        IRType::GetFunction(intType, noParams)
    );
    ASSERT_NE(testFunc, nullptr);

    BasicBlock* entryBlock = testFunc->AddBasicBlock("entry");
    ASSERT_NE(entryBlock, nullptr);

    IRBuilder builder(testLogger);
    builder.SetInsertPoint(entryBlock);

    // Create: t0 = add int 3, 5 (DEAD - result is never used)
    IRConstantInt* const3 = is32Bit ? builder.GetInt32(3) : builder.GetInt64(3);
    IRConstantInt* const5 = is32Bit ? builder.GetInt32(5) : builder.GetInt64(5);
    IRInstruction* deadAddInst = builder.CreateAdd(const3, const5, "dead_add");
    ASSERT_NE(deadAddInst, nullptr);

    // Create: t1 = mul int 2, 3 (DEAD - result is never used)
    IRConstantInt* const2 = is32Bit ? builder.GetInt32(2) : builder.GetInt64(2);
    IRConstantInt* const3b = is32Bit ? builder.GetInt32(3) : builder.GetInt64(3);
    IRInstruction* deadMulInst = builder.CreateMul(const2, const3b, "dead_mul");
    ASSERT_NE(deadMulInst, nullptr);

    // Create: ret int 42 (LIVE - has side effect)
    IRConstantInt* const42 = is32Bit ? builder.GetInt32(42) : builder.GetInt64(42);
    builder.CreateRet(const42);

    // Move constants to module
    auto constants = builder.DumpConstants();
    for (auto& c : constants)
    {
        module.AddConstant(std::move(c));
    }
    builder.ClearConstants();

    // Verify before optimization: should have 3 instructions (add, mul, ret)
    size_t instructionCountBefore = entryBlock->GetInstructions().size();
    EXPECT_EQ(instructionCountBefore, 3);

    // Run DCE pass
    DCEPass dcePass(testLogger);
    bool changed = dcePass.Run(module);

    // Verify: optimization should have made changes
    EXPECT_TRUE(changed) << "DCE should report changes made";

    // Verify: dead instructions should be removed
    size_t instructionCountAfter = entryBlock->GetInstructions().size();
    EXPECT_EQ(instructionCountAfter, 1) << "Only ret instruction should remain";

    // Verify: only ret instruction remains
    IRInstruction* remainingInst = entryBlock->GetTerminator();
    ASSERT_NE(remainingInst, nullptr);
    EXPECT_EQ(remainingInst->GetOpcode(), IROpcode::Ret);

    testLogger.Info("DCEHappyFlow PASSED");
}

// TEST: DCE - Negative Flow - All code is live (no dead code)
TEST(OptimizationTest, DCENegativeFlow) 
{
    EntityLogger testLogger("OPTIMIZATION_TEST");
    testLogger.Info("Testing DCE - Negative Flow");

    // Create an IRModule with NO dead code - all instructions are used
    IRModule module("DCENegTest");
    
    auto intType = GetArchIntType();
    bool is32Bit = TargetArchitecture::Is32Bit();
    
    // Create function: int testFunc(int* ptr) { *ptr = 10; return *ptr; }
    std::vector<std::shared_ptr<const IRType>> params = { IRType::GetPointerTo(intType) };
    IRFunction* testFunc = module.AddFunction(
        "testFunc",
        IRType::GetFunction(intType, params)
    );
    ASSERT_NE(testFunc, nullptr);

    IRArgument* ptrArg = testFunc->AddArgument("ptr", IRType::GetPointerTo(intType));
    ASSERT_NE(ptrArg, nullptr);

    BasicBlock* entryBlock = testFunc->AddBasicBlock("entry");
    ASSERT_NE(entryBlock, nullptr);

    IRBuilder builder(testLogger);
    builder.SetInsertPoint(entryBlock);

    // Create: store int 10, int* %ptr (LIVE - has side effect)
    IRConstantInt* const10 = is32Bit ? builder.GetInt32(10) : builder.GetInt64(10);
    builder.CreateStore(const10, ptrArg);

    // Create: t0 = load i32, i32* %ptr (LIVE - used by ret)
    IRInstruction* loadInst = builder.CreateLoad(ptrArg, "loaded_val");
    ASSERT_NE(loadInst, nullptr);

    // Create: ret i32 t0 (LIVE - terminates function)
    builder.CreateRet(loadInst);

    // Move constants to module
    auto constants = builder.DumpConstants();
    for (auto& c : constants)
    {
        module.AddConstant(std::move(c));
    }
    builder.ClearConstants();

    // Verify before optimization
    size_t instructionCountBefore = entryBlock->GetInstructions().size();
    EXPECT_EQ(instructionCountBefore, 3); // store + load + ret

    // Run DCE pass
    DCEPass dcePass(testLogger);
    bool changed = dcePass.Run(module);

    // Verify: optimization should NOT have made changes (all code is live)
    EXPECT_FALSE(changed) << "DCE should not change code when all instructions are live";

    // Verify: instruction count unchanged
    size_t instructionCountAfter = entryBlock->GetInstructions().size();
    EXPECT_EQ(instructionCountAfter, instructionCountBefore);

    // Verify: all instructions still exist
    bool foundStore = false;
    bool foundLoad = false;
    bool foundRet = false;
    for (const auto& inst : entryBlock->GetInstructions())
    {
        if (inst->GetOpcode() == IROpcode::Store) foundStore = true;
        if (inst->GetOpcode() == IROpcode::Load) foundLoad = true;
        if (inst->GetOpcode() == IROpcode::Ret) foundRet = true;
    }
    EXPECT_TRUE(foundStore) << "Store instruction should remain";
    EXPECT_TRUE(foundLoad) << "Load instruction should remain";
    EXPECT_TRUE(foundRet) << "Ret instruction should remain";

    testLogger.Info("DCENegativeFlow PASSED");
}
