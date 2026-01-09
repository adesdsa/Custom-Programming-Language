#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/semantic_analysis/semantic_analysis_logic.h"
#include "includes/utils/tests_utils.h"
#include "includes/utils/system_utils.h"


// TEST: Happy Flow - Single function
TEST(ParserTest, HappyFlowSmallCodeExample) 
{
    SemanticAnalysisLogicComponent parser(SystemUtils::BuildRelativePath(L"tests/inputs/test3.txt"));
    
}

// TEST: Happy Flow - main function + other function and logic
TEST(ParserTest, HappyFlowFullProgram) 
{
}

// TEST: Negative Flow - Code with logic errors
TEST(ParserTest, NegativeFlowSyntaxErrors) 
{
}
