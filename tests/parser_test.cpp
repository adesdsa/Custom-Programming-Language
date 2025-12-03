#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/parser/parser_logic.h"
#include "includes/utils/tests_utils.h"
#include "includes/utils/system_utils.h"

// TEST: Happy Flow - Single function
TEST(ParserTest, HappyFlowSmallCodeExample) 
{
    ParserLogic parser(SystemUtils::BuildRelativePath("tests/inputs/test3.txt"));
}

// TEST: Happy Flow - main function + other function and logic
TEST(ParserTest, HappyFlowFullProgram) 
{
    ParserLogic parser(SystemUtils::BuildRelativePath("tests/inputs/test4.txt"));

}

// TEST: Negative Flow - Code with syntax errors
TEST(LexerTest, NegativeFlowSyntaxErrors) 
{
    ParserLogic parser(SystemUtils::BuildRelativePath("tests/inputs/test6.txt"));
    EXPECT_THROW(parser.MainFlow(), CompilationAbortException);
}
