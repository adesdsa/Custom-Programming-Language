#pragma once

#include <vector>
#include <string>

#include "parser_consts.h"
#include "token_classifier.h"

#include "includes/tokenizer/token.h"
#include "includes/errors/error_models.h"

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
    bool Append(const Token& currToken, const Token& nextToken);

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
    bool ParseUnidentified(const Token& currToken, const Token& nextToken);

    // The func parses an ACCESS_RIGHTS CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseAccessRights(const Token& currToken, const Token& nextToken);

    // The func parses a SCOPE CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseScope(const Token& currToken, const Token& nextToken);

    // The func parses an IDENTIFIER CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseIdentifier(const Token& currToken, const Token& nextToken);

    // The func parses a FUNC_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseFuncStart(const Token& currToken, const Token& nextToken);

    // The func parses an FILLED_FUNC_DECLARATION CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseFilledFuncDeclaration(const Token& currToken, const Token& nextToken);

    // The func parses an ARROW_FUNC_DECLARATION CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrowFuncDeclaration(const Token& currToken, const Token& nextToken);

    // The func parses a VAR_DECLARATION_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseVarDeclarationStart(const Token& currToken, const Token& nextToken);

    // The func parses an ELIF_CALL, IF_CALL, WHILE_CALL or FOR_CALL CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseParameterKeywordCall(const Token& currToken, const Token& nextToken);

    // The func parses an ELIF_START, IF_START etc CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseParameterKeywordStart(const Token& currToken, const Token& nextToken);

    // The func parses an EXPRESSION_VALUE CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseExpressionValue(const Token& currToken, const Token& nextToken);

    // The func parses an EXPRESSION_OPERATOR CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseExpressionOperator(const Token& currToken, const Token& nextToken);

    // The func parses a POINTER_DECLARATION CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    inline bool ParsePointerDeclaration(const Token& currToken, const Token& nextToken);

    // The func parses a NOT_FUNC_IDENTIFIER CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseNotFuncIdentifier(const Token& currToken, const Token& nextToken);

    // The func parses an ARRAY_ACCESS_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrayAccessStart(const Token& currToken, const Token& nextToken);

    // The func parses an ARRAY_DECLARATION_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrayDeclarationStart(const Token& currToken, const Token& nextToken);

    // The func parses an ARRAY_VALUE_START CommandType.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> finished the Command. False-> otherwise.
    bool ParseArrayValueStart(const Token& currToken, const Token& nextToken);


    bool CompareExpected(const Token& token, std::string sCompare, 
        std::string_view sValidType);


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
    inline bool FoundUnknown(const Token& currToken);


    ///
    ///         Helper Functions
    ///


    // The func appends to the current parameter.
    // @param currToken: The current Token.
    // @param nextToken: The next Token.
    // @return: True-> The param is complete. False-> otherwise.
    inline bool AddToCurrParam(const Token& currToken, const Token& nextToken);
    
    // The func lowers the paranthesesCount, 
    // and handles if there are too many parantheses close and too
    // little opens.
    // @param currToken: The current Token we are added.
    // @return: True-> Too many parantheses closers. False-> Otherwise.
    bool LowerParanthesesCounter(const Token& currToken);

    // The func moves The Command we just worked on (this) 
    // into the parameters so it will be a parameter.
    void MoveThisIntoParam();

    // The func adds an IDENTIFIER to the parameters.
    // @param currToken: The Token that is the IDENTIFIER.
    void AddIdentifier(const Token& currToken);


///
///         Handler Functions
///


    // The func handles function parameters by appending the tokens
    // and checking edge cases.
    // @param currToken: The current Token.
    // @param nextToken: The next Token.
    // @return: True-> The param is complete. False-> otherwise.
    bool HandleFuncLikeParams(const Token& currToken, const Token& nextToken);

    // The func handles the parameters of keywords, e.g.:
    // if(params...), for(params...)
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @return: True-> Finished building the parameter. False-> Otherwise.
    bool HandleKeywordParams(const Token& currToken, const Token& nextToken);

    // The func handles the first time we found paranthes;
    // It handles the first time we enter a func for example.
    // @return: False.
    inline bool HandleFoundParantheses();

    // The func handles the default checks for Commands with parameters.
    // @param currToken: The current Token added right now.
    // @param sParantheses: The parantheses we are looking for.
    // @return: True-> First time we entered the parantheses. False-> Otherwise.
    bool HandleDefaultPossibleParamChecks(const Token& currToken, 
        std::string sParantheses);

    // The func handles the checks regarding the parantheses in arrays.
    // @param currToken: The current Token we are adding.
    // @param nextToken: The next Token we will add.
    // @param sFinishedType: The type that will be changed to if succeeded.
    // @return: True-> Finished the checks. False-> Otherwise.
    bool HandleArrayParanthesesParams(const Token& currToken, const Token& nextToken, 
        std::string_view sFinishedType);

    friend const std::unordered_map<
    std::string_view, bool((Command::*)(const Token&, const Token&))>& getDictHandleTokenPair();
};
