#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/semantic_analysis/semantic_analysis_logic.h"
#include "includes/utils/tests_utils.h"
#include "includes/utils/system_utils.h"


// TEST: Happy Flow - Single function
TEST(SemanticAnalysisTest, HappyFlowSmallCodeExample) 
{
    SemanticAnalysisLogicComponent semanticAnalysisAgent(
        SystemUtils::BuildRelativePath(L"tests/inputs/test3.txt")
    );

    EntityLogger testLogger("GENERAL");

    testLogger.Info("Runs Semantic Analaysis MainFlow");
    const DecoratedAST& resultDecoratedAST = semanticAnalysisComponent.MainFlow();
    
    testLogger.Info("Compare mocked VarTable to the result VarTable");

    /// @note The block are numbered from 0->n (n=number of blocks) their order is from first BLOCK_START in the file until the last.
    VarTableNode* currVarTableNode = resultDecoratedAST.varTable;
    VarMetaData currVarMetaData;
    std::optional<VarMetaData> maybeValue;
    VarMetaData tempMetaDataHolder;

    ASSERT_NE(currVarTableNode, nullptr);  // Check that AST root exists

    /////////////////////////////////////////////////////////////////
    // Global vars checks:                                          |
    // Expected vars: 0                                             |
    // Expected children: 1                                         |
    //      1. Add(let a: int, let b: int) -> void                  |
    /////////////////////////////////////////////////////////////////
    EXPECT_EQ(currVarTableNode->GetVars().size(), 0);
    EXPECT_EQ(currVarTableNode->GetChildren().size(), 1);

    currVarTableNode = &(*currVarTableNode)[0]; // First child from comment at line 18.

    /////////////////////////////////////////////////////////////////
    // First block checks (Add(let a: int, let b: int) -> void):    |
    // Expected vars: 2                                             |
    //      1. let a: int                                           |
    //      2. let b: int                                           |
    // Expected children: 0                                         |
    /////////////////////////////////////////////////////////////////
    EXPECT_EQ(currVarTableNode->GetVars().size(), 2);
    EXPECT_EQ(currVarTableNode->GetChildren().size(), 0);

    // First var: [a, int, default ,let, 4]
    maybeValue = currVarTableNode->FindVar("a");
    ASSERT_TRUE(maybeValue.has_value());
    currVarMetaData = maybeValue.value();
    tempMetaDataHolder = VarMetaData("a", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 4);
    EXPECT_EQ(currVarMetaData, tempMetaDataHolder);

    // Second var: [b, int, default, let, 4]
    maybeValue = currVarTableNode->FindVar("b");
    ASSERT_TRUE(maybeValue.has_value());
    currVarMetaData = maybeValue.value();
    tempMetaDataHolder = VarMetaData("b", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 4);
    EXPECT_EQ(currVarMetaData, tempMetaDataHolder);

    /// Finished comparing VarTable
    
    testLogger.Info("Compare mocked VarTable to the result VarTable");

    // Starts compaing FuncTable
    testLogger.Info("Compare mocked FuncTable to the result FuncTable");

    FuncTable& funcTableResult = resultDecoratedAST.funcTable;
    FuncMetaData currFuncMetaData;
    std::optional<FuncMetaData> maybeValue;
    FuncMetaData mockFuncMetaData;

    ASSERT_NE(funcTableResult, nullptr);

    // Add(let a: int, let b: int) -> void
    maybeValue = resultDecoratedAST->FindVar("Add");
    ASSERT_TRUE(maybeValue.has_value());

    currFuncMetaData = maybeValue.value();
    mockFuncMetaData = FuncMetaData(
        testLogger,
        "Add",
        Type::VOID_TYPE,

    );
    EXPECT_EQ(currFuncMetaData, mockFuncMetaData);

    
}

// TEST: Happy Flow - main function + other function and logic
TEST(SemanticAnalysisTest, HappyFlowFullProgram) 
{
    
}

// TEST: Negative Flow - Code with logic errors
TEST(SemanticAnalysisTest, NegativeFlowSyntaxErrors) 
{
}
