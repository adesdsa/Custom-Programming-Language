#include <gtest/gtest.h>  // gtest FrameWork

#include "includes/parser/parser_logic.h"
#include "includes/utils/tests_utils.h"
#include "includes/utils/system_utils.h"

// TEST: Happy Flow - Single function
TEST(ParserTest, HappyFlowSmallCodeExample) 
{
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/test3.picoc"));
    const AST& ast = parser.MainFlow();
    ASTNode* result_ast = ast.GetRoot();

    ASSERT_NE(result_ast, nullptr);  // Check that AST root exists

    // Initialize helper pointers
    ASTNode* currentNextSibling = result_ast;
    ASTNode* astPtr = result_ast;

    /*
    FINISHED_FUNC_DEFINITION (Add)
        VAR_DECLARATION (a)
        VAR_DECLARATION (b)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_DEFINITION);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VAR_DECLARATION);
    EXPECT_EQ(astPtr->nodeNextSibling->command.GetType(), CommandTypes::VAR_DECLARATION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
        FINISHED_FUNC_CALL (println)
            EXPRESSION(3)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_CALL);
    EXPECT_EQ(astPtr->nodeFirstChild->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_END (})
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_END);
}

// TEST: Happy Flow - main function + other function and logic
TEST(ParserTest, HappyFlowFullProgram) 
{
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/test4.picoc"));
    const AST& ast = parser.MainFlow();
    ASTNode* result_ast = ast.GetRoot();


    ASSERT_NE(result_ast, nullptr);  // Check that AST root exists

    // Initialize helper pointers
    ASTNode* currentNextSibling = result_ast;
    ASTNode* astPtr = result_ast;

    /*
    FINISHED_FUNC_DECLARATION (displayAllPrimeNumbersInRange)
        VAR_DECLARATION (low)
        VAR_DECLARATION (high)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_DECLARATION);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VAR_DECLARATION);
    EXPECT_EQ(astPtr->nodeNextSibling->command.GetType(), CommandTypes::VAR_DECLARATION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    FINISHED_FUNC_DEFINITION (main)
        VOID_COMMAND (void)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_DEFINITION);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VOID_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // VAR_DECLARATION (let)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VAR_DECLARATION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // ASSIGNMENT (=)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::ASSIGNMENT);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;

    // FINISHED_FUNC_CALL (get_int)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_CALL);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // VAR_DECLARATION (let)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VAR_DECLARATION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // ASSIGNMENT (=)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::ASSIGNMENT);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // FINISHED_FUNC_CALL (get_int)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_CALL);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    FINISHED_FUNC_CALL (displayAllPrimeNumbersInRange)
        IDENTIFIER (low)
        IDENTIFIER (high)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_CALL);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER_VALUE);
    EXPECT_EQ(astPtr->nodeNextSibling->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // RETURN (return)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::RETURN);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // EXPRESSION (0)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_END (})
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_END);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    FINISHED_FUNC_DEFINITION (displayAllPrimeNumbersInRange)
        VAR_DECLARATION (low)
        VAR_DECLARATION (high)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_DEFINITION);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VAR_DECLARATION);
    EXPECT_EQ(astPtr->nodeNextSibling->command.GetType(), CommandTypes::VAR_DECLARATION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    WHILE (while)
        EXPRESSION ()
            IDENTIFIER (low)
            IDENTIFIER (high)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::WHILE);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = astPtr->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER_VALUE);
    EXPECT_EQ(astPtr->nodeNextSibling->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // VAR_DECLARATION (let)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VAR_DECLARATION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // ASSIGNMENT (=)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::ASSIGNMENT);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // EXPRESSION (true)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    IF (if)
        EXPRESSION ()
            IDENTIFIER (low)
            IDENTIFIER (low)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IF);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = astPtr->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER_VALUE);
    EXPECT_EQ(astPtr->nodeNextSibling->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // IDENTIFIER (is_prime)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // ASSIGNMENT (=)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::ASSIGNMENT);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // EXPRESSION (false)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_END (})
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_END);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    FOR (for)
        VAR_DECLARATION (let)
        ASSIGNMENT (=)
        EXPRESSION (2)
        EXPRESSION ()
            IDENTIFIER (i)
            IDENTIFIER (low)

        IDENTIFIER (i)
        ASSIGNMENT (=)
        EXPRESSION ()
            IDENTIFIER (i)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FOR);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::VAR_DECLARATION);

    astPtr = astPtr->nodeNextSibling;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::ASSIGNMENT);

    astPtr = astPtr->nodeNextSibling;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = astPtr->nodeNextSibling;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    EXPECT_EQ(astPtr->nodeFirstChild->command.GetType(), CommandTypes::IDENTIFIER_VALUE);
    EXPECT_EQ(astPtr->nodeFirstChild->nodeNextSibling->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = astPtr->nodeNextSibling;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER);

    astPtr = astPtr->nodeNextSibling;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::ASSIGNMENT);

    astPtr = astPtr->nodeNextSibling;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = astPtr->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    IF (if)
        EXPRESSION ()
            IDENTIFIER (low)
            IDENTIFIER (i)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IF);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = astPtr->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER_VALUE);
    EXPECT_EQ(astPtr->nodeNextSibling->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // IDENTIFIER (is_prime)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // ASSIGNMENT (=)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::ASSIGNMENT);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // EXPRESSION (false)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::EXPRESSION);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // CONTROL_STATEMENT (break)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::CONTROL_STATEMENT);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_END (})
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_END);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_END (})
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_END);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    IF (if)
        IDENTIFIER (is_prime)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IF);

    astPtr = currentNextSibling->nodeFirstChild;
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // BLOCK_START ({)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_START);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    /*
    FINISHED_FUNC_CALL (println)
        IDENTIFIER (low)
    */
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::FINISHED_FUNC_CALL);
    EXPECT_EQ(astPtr->nodeFirstChild->command.GetType(), CommandTypes::IDENTIFIER_VALUE);

    astPtr = currentNextSibling->nodeNextSibling;
    currentNextSibling = currentNextSibling->nodeNextSibling;


    // END_OF_COMMAND (;)
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::END_OF_COMMAND);
    astPtr = astPtr->nodeNextSibling;


    // BLOCK_END (})
    EXPECT_EQ(astPtr->command.GetType(), CommandTypes::BLOCK_END);
}

// TEST: Negative Flow - Code with syntax errors
TEST(ParserTest, NegativeFlowSyntaxErrors) 
{
    ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/test6.picoc"));
    EXPECT_THROW(parser.MainFlow(), CompilationAbortException);
}
