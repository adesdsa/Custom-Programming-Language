#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/frontend/parser/parser_logic.h"
#include "includes/frontend/semantic_analysis/semantic_analysis_logic.h"
#include "includes/shared/utils/tests_utils.h"
#include "includes/shared/utils/system_utils.h"


// TEST: Happy Flow - Single function
TEST(SemanticAnalysisTest, HappyFlowSmallCodeExample) 
{
    // Setup: Parse and run semantic analysis
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/small_code_example_file.txt"));
    const AST& ast = parser.MainFlow();
    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);

    EntityLogger testLogger("GENERAL");
    testLogger.Info("Running Semantic Analysis MainFlow");
    
    const DecoratedAST& result = semanticAnalysisComponent.MainFlow();

    /////////////////////////////////////////////////////////////////
    // VarTable Checks                                              |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating VarTable");
    
    const VarTableNode* globalScope = &result.varTable;
    ASSERT_NE(globalScope, nullptr);

    // Global scope: no vars, 1 child (Add function scope)
    EXPECT_EQ(globalScope->GetVars().size(), 0);
    EXPECT_EQ(globalScope->GetChildren().size(), 1);

    // Add function scope: 2 params (a, b)
    const VarTableNode* addFuncScope = &(*globalScope)[0];
    EXPECT_EQ(addFuncScope->GetVars().size(), 2);
    EXPECT_EQ(addFuncScope->GetChildren().size(), 0);

    // Param a
    auto maybeA = addFuncScope->FindVar("a");
    ASSERT_TRUE(maybeA.has_value());
    EXPECT_EQ(maybeA->sName, "a");
    EXPECT_EQ(maybeA->type, Type::INT_TYPE);
    EXPECT_EQ(maybeA->scope, Scope::LET);

    // Param b
    auto maybeB = addFuncScope->FindVar("b");
    ASSERT_TRUE(maybeB.has_value());
    EXPECT_EQ(maybeB->sName, "b");
    EXPECT_EQ(maybeB->type, Type::INT_TYPE);
    EXPECT_EQ(maybeB->scope, Scope::LET);

    /////////////////////////////////////////////////////////////////
    // FuncTable Checks                                             |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating FuncTable");
    
    const FuncTable& funcTable = result.funcTable;

    // Function: Add(let a: int, let b: int) -> void
    FuncMetaData* addFunc = funcTable.FindFunctionByName("Add");
    ASSERT_NE(addFunc, nullptr);
    EXPECT_EQ(addFunc->GetName(), "Add");
    EXPECT_EQ(addFunc->GetRType(), Type::INT_TYPE);
    EXPECT_TRUE(addFunc->IsFuncDefined());
    
    const auto& addParams = addFunc->GetFuncParams();
    EXPECT_EQ(addParams.size(), 2);
    EXPECT_EQ(addParams[0].sName, "a");
    EXPECT_EQ(addParams[0].type, Type::INT_TYPE);
    EXPECT_EQ(addParams[1].sName, "b");
    EXPECT_EQ(addParams[1].type, Type::INT_TYPE);

    testLogger.Info("HappyFlowSmallCodeExample PASSED");
}


// TEST: Happy Flow - main function + other function and logic
TEST(SemanticAnalysisTest, HappyFlowFullProgram) 
{
    // Setup: Parse and run semantic analysis
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/full_program_example_file.txt"));
    const AST& ast = parser.MainFlow();
    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);

    EntityLogger testLogger("GENERAL");
    testLogger.Info("Running Semantic Analysis MainFlow on full program");
    
    const DecoratedAST& result = semanticAnalysisComponent.MainFlow();

    /////////////////////////////////////////////////////////////////
    // VarTable Checks                                              |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating VarTable");
    
    const VarTableNode* globalScope = &result.varTable;
    ASSERT_NE(globalScope, nullptr);

    // Global scope: no vars, 2 children (main + displayAllPrimeNumbersInRange)
    EXPECT_EQ(globalScope->GetVars().size(), 0);
    EXPECT_EQ(globalScope->GetChildren().size(), 2);

    // main function scope - has locals: low, high
    const VarTableNode* mainScope = &(*globalScope)[0];
    EXPECT_EQ(mainScope->GetVars().size(), 2);
    
    auto maybeLow = mainScope->FindVar("low");
    ASSERT_TRUE(maybeLow.has_value());
    EXPECT_EQ(maybeLow->type, Type::INT_TYPE);
    EXPECT_EQ(maybeLow->scope, Scope::LET);

    auto maybeHigh = mainScope->FindVar("high");
    ASSERT_TRUE(maybeHigh.has_value());
    EXPECT_EQ(maybeHigh->type, Type::INT_TYPE);
    EXPECT_EQ(maybeHigh->scope, Scope::LET);

    // displayAllPrimeNumbersInRange function scope - has params: low, high
    const VarTableNode* displayPrimeScope = &(*globalScope)[1];
    
    auto maybeParamLow = displayPrimeScope->FindVar("low");
    ASSERT_TRUE(maybeParamLow.has_value());
    EXPECT_EQ(maybeParamLow->type, Type::INT_TYPE);

    auto maybeParamHigh = displayPrimeScope->FindVar("high");
    ASSERT_TRUE(maybeParamHigh.has_value());
    EXPECT_EQ(maybeParamHigh->type, Type::INT_TYPE);

    /////////////////////////////////////////////////////////////////
    // FuncTable Checks                                             |
    /////////////////////////////////////////////////////////////////
    testLogger.Info("Validating FuncTable");
    
    const FuncTable& funcTable = result.funcTable;

    // Function: main(void) -> int
    FuncMetaData* mainFunc = funcTable.FindFunctionByName("main");
    ASSERT_NE(mainFunc, nullptr);
    EXPECT_EQ(mainFunc->GetName(), "main");
    EXPECT_EQ(mainFunc->GetRType(), Type::INT_TYPE);
    EXPECT_TRUE(mainFunc->IsFuncDefined());
    EXPECT_EQ(mainFunc->GetFuncParams().size(), 0);

    // Function: displayAllPrimeNumbersInRange(low: int, high: int) -> void
    FuncMetaData* displayFunc = funcTable.FindFunctionByName("displayAllPrimeNumbersInRange");
    ASSERT_NE(displayFunc, nullptr);
    EXPECT_EQ(displayFunc->GetName(), "displayAllPrimeNumbersInRange");
    EXPECT_EQ(displayFunc->GetRType(), Type::VOID_TYPE);
    EXPECT_TRUE(displayFunc->IsFuncDeclared());
    EXPECT_TRUE(displayFunc->IsFuncDefined());
    
    const auto& displayParams = displayFunc->GetFuncParams();
    EXPECT_EQ(displayParams.size(), 2);
    EXPECT_EQ(displayParams[0].sName, "low");
    EXPECT_EQ(displayParams[0].type, Type::INT_TYPE);
    EXPECT_EQ(displayParams[1].sName, "high");
    EXPECT_EQ(displayParams[1].type, Type::INT_TYPE);

    testLogger.Info("HappyFlowFullProgram PASSED");
}

// TEST: Negative Flow - Semantic errors
TEST(SemanticAnalysisTest, NegativeFlowSemanticErrors) 
{
    // Setup: Parse and run semantic analysis
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/file_with_logic_errors.txt"));
    const AST& ast = parser.MainFlow();
    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);
    
    EXPECT_THROW(semanticAnalysisComponent.MainFlow(), CompilationAbortException);
}
