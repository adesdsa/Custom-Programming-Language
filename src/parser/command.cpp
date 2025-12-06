#include "includes/parser/command.h"

Command::Command() : 
    m_sType(CommandTypes::UNIDENTIFIED), 
    m_uParameterIndex(0),
    m_bBuildingParam(false), 
    m_bIgnoreCurrToken(false),
    m_bIgnoreEON(false),
    m_iParanthesesCounter(0), 
    m_bFirstParanthesesEnter(false) {}

std::string_view Command::GetType() const 
{
    return this->m_sType;
}

const std::vector<Token>& Command::GetTokens() const 
{
    return this->m_tokens;
}

bool Command::HasParams() const noexcept
{
    return this->m_parameters.size() > 0;
}

std::vector<Command>& Command::GetParams() 
{
    return this->m_parameters;
}

const std::vector<Command>& Command::GetParams() const 
{
    return this->m_parameters;
}

unsigned int Command::GetParanthesesCounter() const 
{
    return this->m_iParanthesesCounter;
}

bool Command::IsExpression() const 
{
    return this->m_bExpression;
}

const std::vector<CompileTimeError>& Command::GetErrors() const 
{
    return this->m_errors;
}

void Command::SetType(std::string_view type) 
{
    this->m_sType = type;
}

void Command::SetExpression(bool indicator) 
{
    this->m_bExpression = indicator;
}

Token& Command::operator[](size_t index) 
{
    return this->m_tokens[index];
}

const Token& Command::operator[](size_t index) const 
{
    return this->m_tokens[index];
}

inline const std::unordered_map<std::string_view, bool((Command::*)(const Token&, const Token&))>& 
getDictHandleTokenPair()
{
    // @brief map that takes `CommandTypes` and call the right parse method.
    // Key: Type of the command represented as `CommandTypes`.
    // Value: The command method the parses given CommandType.
    static const std::unordered_map<std::string_view, bool((Command::*))(const Token&, const Token&)> 
    dictHandleTokenPair = {
        {CommandTypes::UNIDENTIFIED, &Command::ParseUnidentified},
        {CommandTypes::ACCESS_RIGHTS, &Command::ParseAccessRights},
        {CommandTypes::SCOPE, &Command::ParseScope},
        {CommandTypes::IDENTIFIER, &Command::ParseIdentifier},
        {CommandTypes::FUNC_START, &Command::ParseFuncStart},
        {CommandTypes::FILLED_FUNC_DECLARATION, &Command::ParseFilledFuncDeclaration},
        {CommandTypes::ARROW_FUNC_DECLARATION, &Command::ParseArrowFuncDeclaration},
        {CommandTypes::VAR_DECLARATION_START, &Command::ParseVarDeclarationStart},
        {CommandTypes::EXPRESSION_VALUE, &Command::ParseExpressionValue},
        {CommandTypes::EXPRESSION_OPERATOR, &Command::ParseExpressionOperator},
        {CommandTypes::POINTER_DECLARATION, &Command::ParsePointerDeclaration},
        {CommandTypes::NOT_FUNC_IDENTIFIER, &Command::ParseNotFuncIdentifier},
        {CommandTypes::ARRAY_ACCESS_START, &Command::ParseArrayAccessStart},
        {CommandTypes::ARRAY_DECLARATION_START, &Command::ParseArrayDeclarationStart},
        {CommandTypes::ARRAY_VALUE_START, &Command::ParseArrayValueStart}
    };

    return dictHandleTokenPair;
}

bool Command::Append(const Token& currToken, const Token& nextToken)
{
    // reset flags, changes later on following functions calls
    this->m_bIgnoreCurrToken = false; 
    this->m_bIgnoreEON = false;     
    bool bReturnValue = false;
    
    auto& dictHandleTokenPair = getDictHandleTokenPair();
    auto tokenTypeHandler = dictHandleTokenPair.find(this->m_sType);

    if (tokenTypeHandler != dictHandleTokenPair.end())
    {
        bReturnValue = (this->*tokenTypeHandler->second)(currToken, nextToken);
    }

    else if (
                this->m_sType == CommandTypes::ELIF_CALL   || 
                this->m_sType == CommandTypes::IF_CALL     || 
                this->m_sType == CommandTypes::FOR_CALL    ||
                this->m_sType == CommandTypes::WHILE_CALL  ||
                this->m_sType == CommandTypes::SIZEOF_CALL
            ) 
        {
            bReturnValue = ParseParameterKeywordCall(currToken, nextToken);
        }

    else if (
                this->m_sType == CommandTypes::ELIF_START   || 
                this->m_sType == CommandTypes::IF_START     || 
                this->m_sType == CommandTypes::FOR_START    ||
                this->m_sType == CommandTypes::WHILE_START  ||
                this->m_sType == CommandTypes::SIZEOF_START
            ) 
        {
            bReturnValue = ParseParameterKeywordStart(currToken, nextToken);
        }

    else
    {
        return FoundUnknown(currToken);
    }

    if (!this->m_bIgnoreCurrToken) 
    {
        this->m_tokens.push_back(currToken);
    }

    if (TokenClassifier::IsValue(nextToken, ";") && !this->m_bIgnoreEON) 
    {
        bReturnValue = true;
    }

    else if (TokenClassifier::IsValue(nextToken, "{") || TokenClassifier::IsValue(nextToken, "}")) 
    {
        bReturnValue = true;
    }

    return bReturnValue;
}

bool Command::ParseUnidentified(const Token& currToken, const Token& nextToken)
{
    if (
        TokenClassifier::IsType(currToken, TokenTypes::IDENTIFIER) && 
        TokenClassifier::IsValue(nextToken, "=")
    )
    {
        return FinishAndAssignType(CommandTypes::IDENTIFIER);
    }
    else if (TokenClassifier::IsType(currToken, TokenTypes::IDENTIFIER))
    {
        if (TokenClassifier::IsOperator(nextToken))
        {
            this->m_tokens.push_back(
                Token(
                    TokenTypes::HAS_PARAM, 
                    currToken.GetLine(), 
                    currToken.GetColumn()
                )
            );
            this->m_parameters.push_back(Command());
            this->m_parameters[0].m_tokens.push_back(currToken);
            this->m_parameters[0].m_sType = CommandTypes::IDENTIFIER;
            this->m_bIgnoreCurrToken = true;
            this->m_uParameterIndex++;
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else return NoFinishAndAssignType(CommandTypes::IDENTIFIER);
    }
    else if (TokenClassifier::IsType(currToken, TokenTypes::KEYWORD))
    {
        if (TokenClassifier::IsValue(currToken, "void"))
            return FinishAndAssignType(this->m_sType = CommandTypes::VOID_COMMAND);
        else if (TokenClassifier::IsAccessRights(currToken))
            return NoFinishAndAssignType(CommandTypes::ACCESS_RIGHTS);
        else if (TokenClassifier::IsScope(currToken))
            return NoFinishAndAssignType(this->m_sType = CommandTypes::SCOPE);
        else if (TokenClassifier::IsValue(currToken, "else"))
            return FinishAndAssignType(this->m_sType = CommandTypes::ELSE);
        else if (TokenClassifier::IsValue(currToken, "if"))
            return CompareExpected(nextToken, "(", CommandTypes::IF_CALL);
        else if (TokenClassifier::IsValue(currToken, "elif"))
            return CompareExpected(nextToken, "(", CommandTypes::ELIF_CALL);
        else if (TokenClassifier::IsValue(currToken, "for"))
            return CompareExpected(nextToken, "(", CommandTypes::FOR_CALL);
        else if (TokenClassifier::IsValue(currToken, "while"))
            return CompareExpected(nextToken, "(", CommandTypes::WHILE_CALL);
        else if (TokenClassifier::IsValue(currToken, "sizeof"))
            return CompareExpected(nextToken, "(", CommandTypes::SIZEOF_CALL);
        else if (TokenClassifier::IsValue(currToken, "return"))
            return FinishAndAssignType(CommandTypes::RETURN);
        else if (TokenClassifier::IsControlStatement(currToken))
            return FinishAndAssignType(CommandTypes::CONTROL_STATEMENT);
        else if (TokenClassifier::IsValue(currToken, "pass"))
            return FinishAndAssignType(CommandTypes::PASS);
        else return FoundUnknown(currToken);
    }
    else if (TokenClassifier::IsExpressionStart(currToken))
    {
        if (
            TokenClassifier::IsValue(currToken, "-")   || 
            TokenClassifier::IsValue(currToken, "not") ||
            TokenClassifier::IsValue(currToken, "*")   || 
            TokenClassifier::IsValue(currToken, "&")   || 
            TokenClassifier::IsValue(currToken, "(")
        )
        {
            if (TokenClassifier::IsValue(currToken, "("))
            {
                this->m_iParanthesesCounter++;
            }
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
        }
        else return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
    }
    else if (TokenClassifier::IsType(currToken, TokenTypes::PUNCTUATION))
    {
        if (TokenClassifier::IsValue(currToken, ";"))
            return FinishAndAssignType(CommandTypes::END_OF_COMMAND);
        else if (TokenClassifier::IsValue(currToken, "{"))
            return FinishAndAssignType(CommandTypes::BLOCK_START);
        else if (TokenClassifier::IsValue(currToken, "}"))
            return FinishAndAssignType(CommandTypes::BLOCK_END);
        else if (TokenClassifier::IsValue(currToken, "["))
        {
            HandleFoundParantheses();
            return NoFinishAndAssignType(CommandTypes::ARRAY_VALUE_START);
        }
        else return FoundUnknown(currToken);
    }
    else if (TokenClassifier::IsValue(currToken, "="))
        return FinishAndAssignType(CommandTypes::ASSIGNMENT);
    else if (TokenClassifier::IsType(currToken, TokenTypes::EOF_TOKEN))
        return FinishAndAssignType(CommandTypes::EOF_TOKEN);
    else return FoundUnknown(currToken);
}

bool Command::ParseIdentifier(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsValue(currToken, "(") || TokenClassifier::IsValue(currToken, "[")) 
    {
        HandleFoundParantheses();
        this->m_bFirstParanthesesEnter = false;
        if (TokenClassifier::IsValue(currToken, "("))
            return NoFinishAndAssignType(CommandTypes::FUNC_START);
        else return NoFinishAndAssignType(CommandTypes::ARRAY_ACCESS_START);
    }
    else if (currToken.GetValue() == ":")
        return NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    else return FoundUnknown(currToken);
}

bool Command::ParseFuncStart(const Token& currToken, const Token& nextToken)
{
    if (HandleDefaultPossibleParamChecks(currToken, "(")) return false;
    else if (currToken.GetValue() == ")")
    {
        if(LowerParanthesesCounter(currToken)) return true;
        else if (this->m_iParanthesesCounter == 0)
        {
            this->m_bIgnoreCurrToken = false;
            if (TokenClassifier::IsValue(nextToken, "->"))
                return NoFinishAndAssignType(CommandTypes::FILLED_FUNC_DECLARATION);
            else if (TokenClassifier::IsOperator(nextToken))
            {
                this->m_bIgnoreCurrToken = true;
                this->m_tokens.push_back(currToken);
                this->m_sType = CommandTypes::FINISHED_FUNC_CALL;
                MoveThisIntoParam();
                return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
            }
            else return FinishAndAssignType(CommandTypes::FINISHED_FUNC_CALL);
        }
    }

    HandleFuncLikeParams(currToken, nextToken);
    return false;
}

bool Command::ParseFilledFuncDeclaration(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsValue(currToken, "->"))
        return NoFinishAndAssignType(CommandTypes::ARROW_FUNC_DECLARATION);
    else return FoundUnknown(currToken);
}

bool Command::ParseArrowFuncDeclaration(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsType(currToken))
        return FinishAndAssignType(CommandTypes::FINISHED_FUNC_DECLARATION);
    else if (
        TokenClassifier::IsValue(currToken, "unsigned") && 
        TokenClassifier::IsType(nextToken)
    ) 
    {
        return false;
    }
    else return FoundUnknown(currToken);
}

bool Command::ParseAccessRights(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsScope(currToken)) return NoFinishAndAssignType(CommandTypes::SCOPE);
    else if (TokenClassifier::IsType(currToken, TokenTypes::IDENTIFIER))
        return NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    else return FoundUnknown(currToken);
}

bool Command::ParseScope(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsType(currToken, TokenTypes::IDENTIFIER))
        return NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    else return FoundUnknown(currToken);
}

bool Command::ParseVarDeclarationStart(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsType(currToken))
    {
        if (TokenClassifier::IsValue(nextToken, "[")) 
        {
            HandleFoundParantheses();
            return NoFinishAndAssignType(
                CommandTypes::ARRAY_DECLARATION_START);
        }
        if (TokenClassifier::IsValue(nextToken, "*"))
            return NoFinishAndAssignType(CommandTypes::POINTER_DECLARATION);
        else return FinishAndAssignType(CommandTypes::VAR_DECLARATION);
    }
    else return FoundUnknown(currToken);
}

bool Command::ParseParameterKeywordCall(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsValue(currToken, "("))
    {
        HandleFoundParantheses();
        this->m_bFirstParanthesesEnter = false;
        if (this->m_sType == CommandTypes::ELIF_CALL)
            return NoFinishAndAssignType(CommandTypes::ELIF_START);
        else if (this->m_sType == CommandTypes::IF_CALL)
            return NoFinishAndAssignType(CommandTypes::IF_START);
        else if (this->m_sType == CommandTypes::FOR_CALL)
            return NoFinishAndAssignType(CommandTypes::FOR_START);
        else if (this->m_sType == CommandTypes::WHILE_CALL)
            return NoFinishAndAssignType(CommandTypes::WHILE_START);
        else if (this->m_sType == CommandTypes::SIZEOF_CALL)
            return NoFinishAndAssignType(CommandTypes::SIZEOF_START);
        else return FoundUnknown(currToken);
    }
    else return FoundUnknown(currToken);
}

bool Command::ParseParameterKeywordStart(const Token& currToken, const Token& nextToken)
{
    if (HandleDefaultPossibleParamChecks(currToken, "(")) return false;
    else if (TokenClassifier::IsValue(currToken, ")")) 
    {
        if(LowerParanthesesCounter(currToken)) return true;
        if (this->m_iParanthesesCounter == 0)
        {
            this->m_bBuildingParam = false;
            this->m_bIgnoreCurrToken = false;
            if (this->m_sType == CommandTypes::ELIF_START)
                return FinishAndAssignType(CommandTypes::ELIF);
            else if (this->m_sType == CommandTypes::IF_START)
                return FinishAndAssignType(CommandTypes::IF);
            else if (this->m_sType == CommandTypes::FOR_START)
                return FinishAndAssignType(CommandTypes::FOR);
            else if (this->m_sType == CommandTypes::WHILE_START)
                return FinishAndAssignType(CommandTypes::WHILE);
            else if (this->m_sType == CommandTypes::SIZEOF_START)
                return FinishAndAssignType(CommandTypes::SIZEOF);
            else return FoundUnknown(currToken);
        }
    }

    HandleKeywordParams(currToken, nextToken);
    return false;
}

bool Command::ParseExpressionValue(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsOperator(currToken)) return NoFinishAndAssignType(
        CommandTypes::EXPRESSION_OPERATOR);
    else if (TokenClassifier::IsValue(currToken, ")"))
    {
        LowerParanthesesCounter(currToken);
        return NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
    }
    else return FoundUnknown(currToken);
}

bool Command::ParseExpressionOperator(const Token& currToken, const Token& nextToken)
{
    if (this->m_bBuildingParam)
    {
        this->m_bIgnoreCurrToken = true;
        if (AddToCurrParam(currToken, nextToken)) 
        {
            this->m_bBuildingParam = false;
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else return false;
    }
    else if (TokenClassifier::IsValueToken(currToken))
    {
        if (TokenClassifier::IsType(currToken, TokenTypes::IDENTIFIER)) {
            if (TokenClassifier::IsValue(nextToken, "(") || TokenClassifier::IsValue(nextToken, "["))
            {
                this->m_bIgnoreCurrToken = true;
                this->m_bBuildingParam = true;
                AddToCurrParam(currToken, nextToken);
                return false;
            }
            else 
            {
                AddIdentifier(currToken);
                return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
            }
        } 
        else if (TokenClassifier::IsType(currToken, TokenTypes::VALUE)) 
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        else return FoundUnknown(currToken);
    }
    else if (TokenClassifier::IsValue(currToken, "("))
    {
        this->m_iParanthesesCounter++;
        return false;
    }
    else if (TokenClassifier::IsValue(currToken, ")")) return LowerParanthesesCounter(currToken);
    else if (TokenClassifier::IsValue(currToken, "not") || TokenClassifier::IsValue(currToken, "*") ||  
        TokenClassifier::IsValue(currToken, "&")) return false;
    else if (TokenClassifier::IsValue(currToken, "sizeof"))
    {
        this->m_bBuildingParam = true;
        AddToCurrParam(currToken, nextToken);
        return false;
    }
    else return FoundUnknown(currToken);
}

inline bool Command::ParsePointerDeclaration(
    const Token& currToken, const Token& nextToken)
{
    return FinishAndAssignType(CommandTypes::POINTER_DECLARATION);
}

bool Command::ParseNotFuncIdentifier(const Token& currToken, const Token& nextToken)
{
    if (currToken.GetValue() == ":")
        return NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    else return FoundUnknown(currToken);
}

bool Command::ParseArrayAccessStart(const Token& currToken, const Token& nextToken)
{
    if (HandleDefaultPossibleParamChecks(currToken, "[")) return false;
    else if (currToken.GetValue() == "]")
    {
        if (LowerParanthesesCounter(currToken)) return true;
        else if (this->m_iParanthesesCounter == 0)
        {
            this->m_bIgnoreCurrToken = false;
            return FinishAndAssignType(CommandTypes::ARRAY_ACCESS);
        }
    }

    AddToCurrParam(currToken, nextToken);
    return false;
}

bool Command::ParseArrayDeclarationStart(const Token& currToken, const Token& nextToken)
{
    return HandleArrayParanthesesParams(
        currToken, nextToken, CommandTypes::ARRAY_DECLARATION);
}

bool Command::ParseArrayValueStart(const Token& currToken, const Token& nextToken)
{
    return HandleArrayParanthesesParams(
        currToken, nextToken, CommandTypes::ARRAY_VALUE);
}

bool Command::CompareExpected(
    const Token& token, std::string sCompare, std::string_view sValidType)
{
    if (TokenClassifier::IsValue(token, sCompare))
    {
        this->m_sType = sValidType;
        return false;
    }
    else return FoundUnknown(token);
}

inline bool Command::FinishAndAssignType(std::string_view sType)
{
    this->m_sType = sType;
    return true;
}

inline bool Command::NoFinishAndAssignType(std::string_view sType)
{
    this->m_sType = sType;
    return false;
}

inline bool Command::FoundUnknown(const Token& currToken)
{
    if (this->m_errors.size() == 0) this->m_errors.push_back(
        CompileTimeError(ErrorTypes::SYNTAX_ERROR, 
        "Illegal command found",
        currToken.GetLine(), currToken.GetColumn(),
        ""));
    this->m_sType = CommandTypes::UNKNOWN;
    return true;
}


inline bool Command::AddToCurrParam(const Token& currToken, const Token& nextToken)
{
    if (this->m_uParameterIndex == this->m_parameters.size()) 
    {
        this->m_parameters.push_back(Command());
        this->m_tokens.push_back(Token(
            TokenTypes::HAS_PARAM,
            currToken.GetLine(), currToken.GetColumn()));
    }
    if (this->m_parameters[m_uParameterIndex].Append(currToken, nextToken))
    {
        this->m_uParameterIndex++;
        return true;
    }
    return false;
}

bool Command::LowerParanthesesCounter(const Token& currToken)
{
    this->m_iParanthesesCounter--;
    if (this->m_iParanthesesCounter < 0)
    {
        this->m_errors.push_back(CompileTimeError(
            ErrorTypes::SYNTAX_ERROR,
            "Closed too many parantheses",
            currToken.GetLine(), currToken.GetColumn(),
            ""));
        return FoundUnknown(currToken);
    }
    return false;
}

void Command::MoveThisIntoParam()
{
    Command tempSave;
    tempSave.m_tokens = std::move(this->m_tokens);
    tempSave.m_parameters = std::move(this->m_parameters);
    tempSave.m_sType = this->m_sType;
    tempSave.m_uParameterIndex = this->m_uParameterIndex;
    tempSave.m_iParanthesesCounter = this->m_iParanthesesCounter;

    this->m_bBuildingParam = false;
    this->m_bIgnoreEON = false;
    this->m_bExpression = false;
    this->m_iParanthesesCounter = 0;
    this->m_uParameterIndex = 1;
    this->m_tokens.push_back(Token(
        TokenTypes::HAS_PARAM,
        tempSave.m_tokens[0].GetLine(), tempSave.m_tokens[0].GetColumn()));
    this->m_parameters.push_back(tempSave);
}

void Command::AddIdentifier(const Token& currToken)
{
    this->m_bIgnoreCurrToken = true;
    this->m_tokens.push_back(Token(
        TokenTypes::HAS_PARAM,
        currToken.GetLine(), currToken.GetColumn()));
    this->m_parameters.push_back(Command());
    this->m_parameters[this->m_uParameterIndex].m_tokens.push_back(currToken);
    this->m_parameters[this->m_uParameterIndex].m_sType = CommandTypes::IDENTIFIER;
    this->m_bIgnoreCurrToken = true;
    this->m_uParameterIndex++;
}


bool Command::HandleFuncLikeParams(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsValue(currToken, ",")) 
    {
        this->m_tokens.push_back(currToken);
        if (this->m_uParameterIndex < this->m_parameters.size())
            this->m_uParameterIndex++;
        return false;
    }
    else return AddToCurrParam(currToken, nextToken);
}

bool Command::HandleKeywordParams(const Token& currToken, const Token& nextToken)
{
    if (TokenClassifier::IsValue(nextToken, ";")) this->m_bIgnoreEON = true;
    if (TokenClassifier::IsValue(currToken, ";")) 
    {
        this->m_tokens.push_back(currToken);
        if (this->m_uParameterIndex < this->m_parameters.size())
            this->m_uParameterIndex++;
        return false;
    }
    else if (TokenClassifier::IsType(currToken) && this->m_sType == CommandTypes::SIZEOF_START)
    {
        this->m_tokens.push_back(Token(
            TokenTypes::HAS_PARAM,
            currToken.GetLine(), currToken.GetColumn()));
        this->m_parameters.push_back(Command());
        this->m_parameters[this->m_uParameterIndex].m_tokens.push_back(currToken);
        this->m_parameters[this->m_uParameterIndex].m_sType = CommandTypes::VAR_TYPE;
        this->m_uParameterIndex++;
    }
    else return AddToCurrParam(currToken, nextToken);
}

inline bool Command::HandleFoundParantheses()
{
    this->m_bBuildingParam = true;
    this->m_bFirstParanthesesEnter = true;
    this->m_iParanthesesCounter++;
    return false;
}

bool Command::HandleDefaultPossibleParamChecks(
    const Token& currToken, std::string sParantheses)
{
    if (TokenClassifier::IsValue(currToken, sParantheses)) 
    {
        if (this->m_bFirstParanthesesEnter)
        {
            this->m_bFirstParanthesesEnter = !this->m_bFirstParanthesesEnter;
            return true;
        }
        else this->m_iParanthesesCounter++;
    }
    this->m_bIgnoreCurrToken = true;
    return false;
}

bool Command::HandleArrayParanthesesParams(
    const Token& currToken, const Token& nextToken, std::string_view sFinishedType)
{
    if (HandleDefaultPossibleParamChecks(currToken, "[")) return false;
    else if (currToken.GetValue() == "]")
    {
        if (LowerParanthesesCounter(currToken)) return true;
        if (this->m_iParanthesesCounter == 0)
        {
            this->m_bIgnoreCurrToken = false;
            return FinishAndAssignType(sFinishedType);
        }
    }

    HandleFuncLikeParams(currToken, nextToken);
    return false;
}