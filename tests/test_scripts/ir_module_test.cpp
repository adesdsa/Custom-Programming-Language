#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/frontend/parser/parser_logic.h"
#include "includes/frontend/semantic_analysis/semantic_analysis_logic.h"
#include "includes/backend/codegen/ir/ir_generator.h"
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
