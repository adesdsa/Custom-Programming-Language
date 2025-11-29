#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "includes/tokenizer/token.h"

///todo
///             This Class Will Be Changed To Decorated Tokens And Then Will Make Command Class
/// if it's the eof we give the curr and next Token as the same eof token

namespace CommandTypes
{
    /// 
    ///             The next CommandTypes are the finished CommandTypes
    ///
    inline constexpr std::string_view IDENTIFIER                    = "IDENTIFIER";
    inline constexpr std::string_view UNKNOWN                       = "UNKNOWN";
    inline constexpr std::string_view END_OF_COMMAND                = "END_OF_COMMAND";
    inline constexpr std::string_view BLOCK_START                   = "BLOCK_START";
    inline constexpr std::string_view BLOCK_END                     = "BLOCK_END";
    inline constexpr std::string_view ELSE                          = "ELSE";
    inline constexpr std::string_view RETURN                        = "RETURN";
    inline constexpr std::string_view ASSIGNMENT                    = "ASSIGNMENT";
    inline constexpr std::string_view VOID_COMMAND                  = "VOID_COMMAND";
    inline constexpr std::string_view POINTER_DECLARATION           = "POINTER_DECLARATION";
    inline constexpr std::string_view CONTROL_STATEMENT             = "CONTROL_STATEMENT";
    inline constexpr std::string_view VAR_DECLARATION               = "VAR_DECLARATION";
    inline constexpr std::string_view EOF_TOKEN                     = "EOF_TOKEN";
    inline constexpr std::string_view EXPRESSION                    = "EXPRESSION";
    inline constexpr std::string_view PASS                          = "PASS";
    inline constexpr std::string_view ARRAY_VALUE                   = "ARRAY_VALUE";
    inline constexpr std::string_view POINTER_USE                   = "POINTER_USE";


    /// 
    ///             The next CommandTypes are the finished CommandTypes that need to be reformed in CommandLine
    ///
    inline constexpr std::string_view ELIF                          = "ELIF";
    inline constexpr std::string_view IF                            = "IF";
    inline constexpr std::string_view FOR                           = "FOR";
    inline constexpr std::string_view WHILE                         = "WHILE";
    inline constexpr std::string_view SIZEOF                        = "SIZEOF";
    inline constexpr std::string_view FINISHED_FUNC_DECLARATION     = "FINISHED_FUNC_DECLARATION";
    inline constexpr std::string_view FINISHED_FUNC_CALL            = "FINISHED_FUNC_CALL";
    inline constexpr std::string_view ARRAY_ACCESS                  = "ARRAY_ACCESS";
    inline constexpr std::string_view ARRAY_DECLARATION             = "ARRAY_DECLARATION";

    inline constexpr std::string_view EXPRESSION_VALUE              = "EXPRESSION_VALUE";
    inline constexpr std::string_view EXPRESSION_OPERATOR           = "EXPRESSION_OPERATOR";
    inline constexpr std::string_view VAR_TYPE                      = "VAR_TYPE";


    /// 
    ///             The next CommandTypes are the non finished CommandTypes
    ///
    inline constexpr std::string_view UNIDENTIFIED                  = "UNIDENTIFIED";
    inline constexpr std::string_view FUNC_START                    = "FUNC_START";
    inline constexpr std::string_view FILLED_FUNC_DECLARATION       = "FILLED_FUNC_DECLARATION";
    inline constexpr std::string_view ARROW_FUNC_DECLARATION        = "ARROW_FUNC_DECLARATION";
    inline constexpr std::string_view ACCESS_RIGHTS                 = "ACCESS_RIGHTS";
    inline constexpr std::string_view SCOPE                         = "SCOPE";
    inline constexpr std::string_view VAR_DECLARATION_START         = "VAR_DECLARATION_START";
    inline constexpr std::string_view ELIF_CALL                     = "ELIF_CALL";
    inline constexpr std::string_view IF_CALL                       = "IF_CALL";
    inline constexpr std::string_view ELIF_START                    = "ELIF_START";
    inline constexpr std::string_view IF_START                      = "IF_START";
    inline constexpr std::string_view FOR_CALL                      = "FOR_CALL";
    inline constexpr std::string_view WHILE_CALL                    = "WHILE_CALL";
    inline constexpr std::string_view FOR_START                     = "FOR_START";
    inline constexpr std::string_view WHILE_START                   = "WHILE_START";
    inline constexpr std::string_view SIZEOF_CALL                   = "SIZEOF_CALL";
    inline constexpr std::string_view SIZEOF_START                  = "SIZEOF_START";
    inline constexpr std::string_view ARRAY_ACCESS_START            = "ARRAY_ACCESS_START";
    inline constexpr std::string_view NOT_FUNC_IDENTIFIER           = "NOT_FUNC_IDENTIFIER";
    inline constexpr std::string_view ARRAY_DECLARATION_START       = "ARRAY_DECLARATION_START";
    inline constexpr std::string_view ARRAY_VALUE_START             = "ARRAY_VALUE_START";
}

class Command 
{
public:
    Command();

    // The func adds Tokens inorder to create a full legal command.
    // A Command is the smallest operation that can happen e.g. = is ASSIGNMENT
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we are adding used for checking.
    // @return: True-> Finished the Command. False-> otherwise.
    bool Append(Token& currToken, Token& nextToken);

    // Getters

    std::string_view GetType() const;
    const std::vector<Token>& GetTokens() const;
    bool HasParams() const noexcept;
    std::vector<Command>& GetParams();
    const std::vector<Command>& GetParams() const;
    unsigned int GetParanthesesCounter() const;
    bool IsExpression() const;

    // Setters

    void SetType(std::string_view type);
    void SetExpression(bool indicator);

    // Operators
    Token& operator[](size_t index);
    const Token& operator[](size_t index) const;

private:
    std::vector<Token> m_tokens;            // Used for saving the tokens that the Command is made of.
    std::vector<Command> m_parameters;      // Used for saving the parameters of a command (for example the func params).
    std::string_view m_sType;               // The Command's type.
    bool m_bBuildingParam;                  // Used to indicate if we are building a parameter right now.
    bool m_bIgnoreCurrToken;                // Used to indicate if we don't need to add the current token to the tokens vector.
    bool m_bIgnoreEON;                      // Used to indicate that we need to ignore end of line.
    bool m_bExpression;                     // Used to indicate if this Command is an expression.
    bool m_bFirstParanthesesEnter;          // Used to indicate if this is the first time we entered the parantheses
    int m_iParanthesesCounter;     // Used for counting how many parantheses we saw (that way for example we know when we finished getting all of the func's params(when the counter is 0)).
    size_t m_uParameterIndex;               // Saves the current parameter index.


    ///
    ///         Parser Functions
    ///

    // The func parses an UNIDENTIFIED CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseUnidentified(Token& currToken, Token& nextToken);

    // The func parses an ACCESS_RIGHTS CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseAccessRights(Token& currToken, Token& nextToken);

    // The func parses a SCOPE CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseScope(Token& currToken, Token& nextToken);

    // The func parses an IDENTIFIER CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseIdentifier(Token& currToken, Token& nextToken);

    // The func parses a FUNC_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseFuncStart(Token& currToken, Token& nextToken);

    // The func parses an FILLED_FUNC_DECLARATION CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseFilledFuncDeclaration(Token& currToken, Token& nextToken);

    // The func parses an ARROW_FUNC_DECLARATION CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrowFuncDeclaration(Token& currToken, Token& nextToken);

    // The func parses a VAR_DECLARATION_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseVarDeclarationStart(Token& currToken, Token& nextToken);

    // The func parses an ELIF_CALL, IF_CALL, WHILE_CALL or FOR_CALL CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseParameterKeywordCall(Token& currToken, Token& nextToken);

    // The func parses an ELIF_START, IF_START, WHILE_START or FOR_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseParameterKeywordStart(Token& currToken, Token& nextToken);

    // The func parses an EXPRESSION_VALUE CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseExpressionValue(Token& currToken, Token& nextToken);

    // The func parses an EXPRESSION_OPERATOR CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseExpressionOperator(Token& currToken, Token& nextToken);

    // The func parses a POINTER_DECLARATION CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    inline bool ParsePointerDeclaration(Token& currToken, Token& nextToken);

    // The func parses a NOT_FUNC_IDENTIFIER CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseNotFuncIdentifier(Token& currToken, Token& nextToken);

    // The func parses an ARRAY_ACCESS_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrayAccessStart(Token& currToken, Token& nextToken);

    // The func parses an ARRAY_DECLARATION_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrayDeclarationStart(Token& currToken, Token& nextToken);

    // todo: add docs
    bool ParseArrayValueStart(Token& currToken, Token& nextToken);


    ///
    ///         Classification Checker Functions
    ///


    // The func compares if the Token's value equals to the string.
    // @param token: The token we are comparing.
    // @param sCompare: The string we compare.
    // @return: True-> the Token's value equals the string. False-> otherwise.
    inline bool IsValue(Token& token, std::string sCompare);

    // The func compares if the Token's type equals to the string_view(The type).
    // @param token: The token we are comparing.
    // @param sCompare: The string_view which is a TokenType we compare.
    // @return: True-> the Token's type equals the given type. False-> otherwise.
    inline bool IsType(Token& token, std::string_view sCompare);

    // The func compares if the Token's value equals to the string, if it does then we change the type to the given type, else we change to unknown.
    // @param token: The token we are comparing.
    // @param sCompare: The string we compare the Token's value to.
    // @param sValidType: The string_view which is what we change the CommandType to if the compare works.
    // @return: False-> the Token's type equals the given type. True-> otherwise.
    bool CompareExpected(Token& token, std::string sCompare, std::string_view sValidType);

    // The func checks if the given token is an Access right.
    // @param token: The token we are checking.
    // @return: True-> the Token is an Access Right. False-> otherwise.
    bool IsAccessRights(Token& token);

    // The func checks if the given token is a Scope.
    // @param token: The token we are checking.
    // @return: True-> the Token is a Scope. False-> otherwise.
    bool IsScope(Token& token);

    // The func checks if the given token is an expression start.
    // @param token: The token we are checking.
    // @return: True-> the Token is an expression start. False-> otherwise.
    bool IsExpressionStart(Token& token);

    // The func checks if the given token is a legal expression value.
    // @param token: The token we are checking.
    // @return: True-> the Token is a legal expression value. False-> otherwise.
    inline bool IsValueToken(Token& token);

    // The func checks if the given token is an expression operator.
    // @param token: The token we are checking.
    // @return: True-> the Token is an expression operator. False-> otherwise.
    bool IsOperator(Token& token);

    // The func checks if the given token is a Var Type.
    // @param token: The token we are checking.
    // @return: True-> the Token is a Var Type. False-> otherwise.
    inline bool IsType(Token& token);

    // The func checks if the given token is a control statement.
    // @param token: The token we are checking.
    // @return: True-> the Token is a control statement. False-> otherwise.
    bool IsControlStatement(Token& token);


///
///         Parsing finish Functions
///


    // The func assigns the given CommandType and return true.
    // @param sType: The CommandType we assign.
    // @return: True.
    inline bool FinishAndAssignType(std::string_view sType);

    // The func assigns the given CommandType and return false.
    // @param sType: The CommandType we assign.
    // @return: False.
    inline bool NoFinishAndAssignType(std::string_view sType);

    // The func makes the CommandType UNKNOWN and returns true.
    // @return: True.
    inline bool FoundUnknown();


///
///         Helper Functions
///


    // The func appends to the current parameter.
    // @param currToken: The current Token.
    // @param nextToken: The next Token.
    // @return: True-> The param is complete. False-> otherwise.
    inline bool AddToCurrParam(Token& currToken, Token& nextToken);
    
    bool LowerParanthesesCounter();

    void MoveThisIntoParam();


///
///         Handler Functions
///


    // The func handles function parameters by appending the tokens and checking edge cases.
    // @param currToken: The current Token.
    // @param nextToken: The next Token.
    // @return: True-> The param is complete. False-> otherwise.
    bool HandleFuncLikeParams(Token& currToken, Token& nextToken);

    // todo: docs
    bool HandleKeywordParams(Token& currToken, Token& nextToken);

    inline bool HandleFoundParantheses();

    bool HandleDefaultPossibleParamChecks(Token& currToken, std::string sParantheses);

    bool HandleArrayParanthesesParams(Token& currToken, Token& nextToken, std::string_view sFinishedType);
};
