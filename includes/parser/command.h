#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "includes/tokenizer/token.h"
#include "includes/errors/error_models.h"


namespace CommandTypes
{
    /// 
    ///             The next CommandTypes are the finished CommandTypes.
    ///
    inline constexpr std::string_view IDENTIFIER                    = "IDENTIFIER";
    inline constexpr std::string_view UNKNOWN                       = "UNKNOWN";
    inline constexpr std::string_view END_OF_COMMAND                = "END_OF_COMMAND";     // ;
    inline constexpr std::string_view BLOCK_START                   = "BLOCK_START";        // {
    inline constexpr std::string_view BLOCK_END                     = "BLOCK_END";          // }
    inline constexpr std::string_view ELSE                          = "ELSE";
    inline constexpr std::string_view RETURN                        = "RETURN";
    inline constexpr std::string_view ASSIGNMENT                    = "ASSIGNMENT";         // =
    inline constexpr std::string_view VOID_COMMAND                  = "VOID_COMMAND";
    inline constexpr std::string_view CONTROL_STATEMENT             = "CONTROL_STATEMENT";
    inline constexpr std::string_view VAR_DECLARATION               = "VAR_DECLARATION";
    inline constexpr std::string_view EOF_TOKEN                     = "EOF_TOKEN";
    inline constexpr std::string_view EXPRESSION                    = "EXPRESSION";         // any mathematic or boolean operation
    inline constexpr std::string_view PASS                          = "PASS";
    inline constexpr std::string_view ARRAY_VALUE                   = "ARRAY_VALUE";
    inline constexpr std::string_view IDENTIFIER_VALUE              = "IDENTIFIER_VALUE";   // IDENTIFIER call where the values is used


    /// 
    ///             The next CommandTypes are the finished CommandTypes that need to be reformed in FullCommand.
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
    inline constexpr std::string_view VAR_TYPE                      = "VAR_TYPE";           // int, bool...


    ///
    ///             The next CommandTypes must be reformed and aren't finished yet.
    ///
    inline constexpr std::string_view EXPRESSION_VALUE              = "EXPRESSION_VALUE";
    inline constexpr std::string_view EXPRESSION_OPERATOR           = "EXPRESSION_OPERATOR";


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

// The class represents a set of Tokens that create the most basic Command.
// E.G. VAR_DECLARATION, ASSIGNMENT, EXPRESSION, WHILE
// To see all of the types please check out CommandTypes namespace above.
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

    // The func returns the Command's type.
    std::string_view GetType() const;
    // The func returns the Token's of the Command.
    const std::vector<Token>& GetTokens() const;
    // Does the Command have parameters.
    bool HasParams() const noexcept;
    // Get the Command's parameters.
    std::vector<Command>& GetParams();
    // Get the Command's parameters as a const.
    const std::vector<Command>& GetParams() const;
    // Get the paranthesesCounter of the Command.
    // Used to check if not all parantheses are closed
    unsigned int GetParanthesesCounter() const;
    // True-> The Command is a legal expression e.g.: legal FUNC_CALL, legal EXPRESSION.
    // False-> otherwise.
    bool IsExpression() const;
    // Get the vector of all the encountered errors.
    const std::vector<CompileTimeError>& GetErrors() const;

    // Setters

    // Set the Command's types.
    void SetType(std::string_view type);
    // Set if the Command is an expression e.g.:
    // legal EXPRESSION, legal FUNC_CALL...
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
    int m_iParanthesesCounter;              // Used for counting how many parantheses we saw (that way for example we know when we finished getting all of the func's params(when the counter is 0)).
    size_t m_uParameterIndex;               // Saves the current parameter index.
    std::vector<CompileTimeError> m_errors;


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

    // The func parses an ELIF_START, IF_START etc CommandType.
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

    // The func parses an ARRAY_VALUE_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrayValueStart(Token& currToken, Token& nextToken);


    ///
    ///         Classification Checker Functions
    ///


    // The func compares if the Token's value equals to the string.
    // @param token: The token we are comparing.
    // @param sCompare: The string we compare.
    // @return: True-> the Token's value equals the string. False-> otherwise.
    inline bool IsValue(Token& token, std::string sCompare);

    // The func compares if the Token's type equals to a TokenType.
    // @param token: The token we are comparing.
    // @param sCompare: The string_view which is a TokenType we compare.
    // @return: True-> the Token's type equals the given type.
    // False-> otherwise.
    inline bool IsType(Token& token, std::string_view sCompare);

    // The func compares if the Token's value equals to the string, 
    // if it does then we change the type to the given type, 
    // else we change to unknown.
    // @param token: The token we are comparing.
    // @param sCompare: The string we compare the Token's value to.
    // @param sValidType: The string_view which is what we change the 
    // CommandType to if the compare works.
    // @return: False-> the Token's type equals the given type. True-> otherwise.
    bool CompareExpected(Token& token, std::string sCompare, 
        std::string_view sValidType);

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
    // @return: True-> the Token is a legal expression value. 
    // False-> otherwise.
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
    inline bool FoundUnknown(Token& currToken);


///
///         Helper Functions
///


    // The func appends to the current parameter.
    // @param currToken: The current Token.
    // @param nextToken: The next Token.
    // @return: True-> The param is complete. False-> otherwise.
    inline bool AddToCurrParam(Token& currToken, Token& nextToken);
    
    // The func lowers the paranthesesCount, 
    // and handles if there are too many parantheses close and too
    // little opens.
    // @param currToken: The current Token we are added.
    // @return: True-> Too many parantheses closers. False-> Otherwise.
    bool LowerParanthesesCounter(Token& currToken);

    // The func moves The Command we just worked on (this) 
    // into the parameters so it will be a parameter.
    void MoveThisIntoParam();

    // The func adds an IDENTIFIER to the parameters.
    // @param currToken: The Token that is the IDENTIFIER.
    void AddIdentifier(Token& currToken);


///
///         Handler Functions
///


    // The func handles function parameters by appending the tokens
    // and checking edge cases.
    // @param currToken: The current Token.
    // @param nextToken: The next Token.
    // @return: True-> The param is complete. False-> otherwise.
    bool HandleFuncLikeParams(Token& currToken, Token& nextToken);

    // The func handles the parameters of keywords, e.g.:
    // if(params...), for(params...)
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> Finished building the parameter. False-> Otherwise.
    bool HandleKeywordParams(Token& currToken, Token& nextToken);

    // The func handles the first time we found paranthes;
    // It handles the first time we enter a func for example.
    // @return: False.
    inline bool HandleFoundParantheses();

    // The func handles the default checks for Commands with parameters.
    // @param currToken: The current Token added right now.
    // @param sParantheses: The parantheses we are looking for.
    // @return: True-> First time we entered the parantheses. False-> Otherwise.
    bool HandleDefaultPossibleParamChecks(Token& currToken, 
        std::string sParantheses);

    // The func handles the checks regarding the parantheses in arrays.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @param sFinishedType: The type that will be changed to if succeeded.
    // @return: True-> Finished the checks. False-> Otherwise.
    bool HandleArrayParanthesesParams(Token& currToken, Token& nextToken, 
        std::string_view sFinishedType);
};
