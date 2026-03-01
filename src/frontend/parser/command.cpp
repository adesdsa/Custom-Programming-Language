#include "includes/frontend/parser/command.h"


Command::Command() : 
    m_sType(CommandTypes::UNIDENTIFIED), 
    m_uParameterIndex(0),
    m_bBuildingParam(false),
    m_bIgnoreCurrToken(false), 
    m_bIgnoreEON(false),
    m_bExpression(false), 
    m_iParanthesesCounter(0), 
    m_bFirstParanthesesEnter(false)
{}

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

void Command::SetValue(Token token)
{
    this->m_tokens.clear();
    this->m_tokens.push_back(token);
}

Token& Command::operator[](size_t index)
{
    return this->m_tokens[index];
}

const Token& Command::operator[](size_t index) const
{
    return this->m_tokens[index];
}


bool Command::Append(Token& currToken, Token& nextToken)
{
    this->m_bIgnoreCurrToken = false;
    this->m_bIgnoreEON = false;
    bool bReturnValue = false;

    if (this->m_sType == CommandTypes::UNIDENTIFIED)
    {
        bReturnValue = this->ParseUnidentified(currToken, nextToken);
    }
    else if (this->m_sType == CommandTypes::ACCESS_RIGHTS)
    {
        bReturnValue = this->ParseAccessRights(currToken);
    }
    else if (this->m_sType == CommandTypes::SCOPE)
    {
        bReturnValue = this->ParseScope(currToken);
    }
    else if (this->m_sType == CommandTypes::IDENTIFIER_VALUE)
    {
        bReturnValue = this->ParseIdentifier(currToken);
    }
    else if (this->m_sType == CommandTypes::FUNC_START)
    {
        bReturnValue = this->ParseFuncStart(currToken, nextToken);
    }
    else if (this->m_sType == CommandTypes::FILLED_FUNC_DECLARATION)
    {
        bReturnValue = this->ParseFilledFuncDeclaration(currToken);
    }
    else if (this->m_sType == CommandTypes::ARROW_FUNC_DECLARATION)
    {
        bReturnValue = this->ParseArrowFuncDeclaration(currToken, nextToken);
    }
    else if (this->m_sType == CommandTypes::VAR_DECLARATION_START)
    {
        bReturnValue = this->ParseVarDeclarationStart(currToken, nextToken);
    }
    else if (
        this->m_sType == CommandTypes::ELIF_CALL  || 
        this->m_sType == CommandTypes::IF_CALL    || 
        this->m_sType == CommandTypes::FOR_CALL   ||
        this->m_sType == CommandTypes::WHILE_CALL ||
        this->m_sType == CommandTypes::SIZEOF_CALL
    )
    {
        bReturnValue = this->ParseParameterKeywordCall(currToken);
    }
    else if (
        this->m_sType == CommandTypes::ELIF_START  ||
        this->m_sType == CommandTypes::IF_START    ||
        this->m_sType == CommandTypes::FOR_START   ||
        this->m_sType == CommandTypes::WHILE_START ||
        this->m_sType == CommandTypes::SIZEOF_START
    )
    {
        bReturnValue = this->ParseParameterKeywordStart(currToken, nextToken);
    }
    else if (this->m_sType == CommandTypes::EXPRESSION_VALUE)
    {
        bReturnValue = this->ParseExpressionValue(currToken);
    }
    else if (this->m_sType == CommandTypes::EXPRESSION_OPERATOR)
    {
        bReturnValue = this->ParseExpressionOperator(currToken, nextToken);
    }
    else if (this->m_sType == CommandTypes::NOT_FUNC_IDENTIFIER)
    {
        bReturnValue = this->ParseNotFuncIdentifier(currToken);
    }
    else if (this->m_sType == CommandTypes::ARRAY_ACCESS_START)
    {
        bReturnValue = this->ParseArrayAccessStart(currToken, nextToken);
    }
    else if (this->m_sType == CommandTypes::ARRAY_DECLARATION_START)
    {
        bReturnValue = this->ParseArrayDeclarationStart(currToken, nextToken);
    }
    else if (this->m_sType == CommandTypes::ARRAY_VALUE_START)
    {
        bReturnValue = this->ParseArrayValueStart(currToken, nextToken);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }

    if (!this->m_bIgnoreCurrToken)
    {
        this->m_tokens.push_back(currToken);
    }
    if (this->IsValue(nextToken, ";") && !this->m_bIgnoreEON)
    {
        bReturnValue = true;
    }
    if (this->IsValue(nextToken, "{") || this->IsValue(nextToken, "}"))
    {
        bReturnValue = true;
    }

    return bReturnValue;
}

bool Command::ParseUnidentified(Token& currToken, Token& nextToken)
{
    if (this->IsType(currToken, TokenTypes::IDENTIFIER) && this->IsValue(nextToken, "="))
    {
        return this->FinishAndAssignType(CommandTypes::IDENTIFIER);
    }
    else if (this->IsType(currToken, TokenTypes::IDENTIFIER))
    {
        if (this->IsOperator(nextToken))
        {
            this->AddIdentifier(currToken);
            return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else
        {
            return this->NoFinishAndAssignType(CommandTypes::IDENTIFIER_VALUE);
        }
    }
    else if (this->IsType(currToken, TokenTypes::KEYWORD))
    {
        if (this->IsValue(currToken, "void"))
        {
            return this->FinishAndAssignType(this->m_sType = CommandTypes::VOID_COMMAND);
        }
        else if (this->IsAccessRights(currToken))
        {
            return this->NoFinishAndAssignType(CommandTypes::ACCESS_RIGHTS);
        }
        else if (this->IsScope(currToken))
        {
            return this->NoFinishAndAssignType(this->m_sType = CommandTypes::SCOPE);
        }
        else if (this->IsValue(currToken, "else"))
        {
            return this->FinishAndAssignType(this->m_sType = CommandTypes::ELSE);
        }
        else if (this->IsValue(currToken, "if"))
        {
            return this->CompareExpected(nextToken, "(", CommandTypes::IF_CALL);
        }
        else if (this->IsValue(currToken, "elif"))
        {
            return this->CompareExpected(nextToken, "(", CommandTypes::ELIF_CALL);
        }
        else if (this->IsValue(currToken, "for"))
        {
            return this->CompareExpected(nextToken, "(", CommandTypes::FOR_CALL);
        }
        else if (this->IsValue(currToken, "while"))
        {
            return this->CompareExpected(nextToken, "(", CommandTypes::WHILE_CALL);
        }
        else if (this->IsValue(currToken, "sizeof"))
        {
            return this->CompareExpected(nextToken, "(", CommandTypes::SIZEOF_CALL);
        }
        else if (this->IsValue(currToken, "return"))
        {
            return this->FinishAndAssignType(CommandTypes::RETURN);
        }
        else if (this->IsControlStatement(currToken))
        {
            return this->FinishAndAssignType(CommandTypes::CONTROL_STATEMENT);
        }
        else if (this->IsValue(currToken, "pass"))
        {
            return this->FinishAndAssignType(CommandTypes::PASS);
        }
        else if (this->IsValue(currToken, "not"))
        {
            return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
        }
        else
        {
            return this->FoundUnknown(currToken);
        }
    }
    else if (this->IsExpressionStart(currToken))
    {
        if (this->IsValue(currToken, "-") || this->IsValue(currToken, "not") ||
            this->IsValue(currToken, "("))
        {
            if (this->IsValue(currToken, "("))
            {
                this->m_iParanthesesCounter++;
            }
            return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
        }
        else
        {
            return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
    }
    else if (this->IsType(currToken, TokenTypes::PUNCTUATION))
    {
        if (this->IsValue(currToken, ";"))
        {
            return this->FinishAndAssignType(CommandTypes::END_OF_COMMAND);
        }
        else if (this->IsValue(currToken, "{"))
        {
            return this->FinishAndAssignType(CommandTypes::BLOCK_START);
        }
        else if (this->IsValue(currToken, "}"))
        {
            return this->FinishAndAssignType(CommandTypes::BLOCK_END);
        }
        else if (this->IsValue(currToken, "["))
        {
            this->HandleFoundParantheses();
            return this->NoFinishAndAssignType(CommandTypes::ARRAY_VALUE_START);
        }
        else
        {
            return this->FoundUnknown(currToken);
        }
    }
    else if (this->IsValue(currToken, "="))
    {
        return this->FinishAndAssignType(CommandTypes::ASSIGNMENT);
    }
    else if (this->IsType(currToken, TokenTypes::EOF_TOKEN))
    {
        return this->FinishAndAssignType(CommandTypes::EOF_TOKEN);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseIdentifier(Token& currToken)
{
    if (this->IsValue(currToken, "(") || this->IsValue(currToken, "[")) 
    {
        this->HandleFoundParantheses();
        this->m_bFirstParanthesesEnter = false;

        if (this->IsValue(currToken, "("))
        {
            return this->NoFinishAndAssignType(CommandTypes::FUNC_START);
        }
        else
        {
            return this->NoFinishAndAssignType(CommandTypes::ARRAY_ACCESS_START);
        }
    }
    else if (currToken.GetValue() == ":")
    {
        return this->NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseFuncStart(Token& currToken, Token& nextToken)
{
    if (this->HandleDefaultPossibleParamChecks(currToken, "("))
    {
        return false;
    }
    else if (currToken.GetValue() == ")")
    {
        if (this->LowerParanthesesCounter(currToken))
        {
            return true;
        }
        else if (this->m_iParanthesesCounter == 0)
        {
            this->m_bBuildingParam = false;
            this->m_bIgnoreCurrToken = false;

            if (this->IsValue(nextToken, "->"))
            {
                return this->NoFinishAndAssignType(CommandTypes::FILLED_FUNC_DECLARATION);
            }
            else if (this->IsOperator(nextToken))
            {
                this->m_bIgnoreCurrToken = true;
                this->m_tokens.push_back(currToken);
                this->m_sType = CommandTypes::FINISHED_FUNC_CALL;
                this->MoveThisIntoParam();
                return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
            }
            else
            {
                return this->FinishAndAssignType(CommandTypes::FINISHED_FUNC_CALL);
            }
        }
    }

    this->HandleFuncLikeParams(currToken, nextToken);
    return false;
}

bool Command::ParseFilledFuncDeclaration(Token& currToken)
{
    if (this->IsValue(currToken, "->"))
    {
        return this->NoFinishAndAssignType(CommandTypes::ARROW_FUNC_DECLARATION);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseArrowFuncDeclaration(Token& currToken, Token& nextToken)
{
    if (this->IsType(currToken))
    {
        return this->FinishAndAssignType(CommandTypes::FINISHED_FUNC_DECLARATION);
    }
    else if (this->IsValue(currToken, "unsigned") && this->IsType(nextToken))
    {
        return false;
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseAccessRights(Token& currToken)
{
    if (this->IsType(currToken, TokenTypes::IDENTIFIER))
    {
        return this->NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseScope(Token& currToken)
{
    if (this->IsAccessRights(currToken))
    {
        return this->NoFinishAndAssignType(CommandTypes::ACCESS_RIGHTS);
    }
    else if (this->IsType(currToken, TokenTypes::IDENTIFIER))
    {
        return this->NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseVarDeclarationStart(Token& currToken, Token& nextToken)
{
    if (this->IsType(currToken))
    {
        if (this->IsValue(nextToken, "[")) 
        {
            this->HandleFoundParantheses();
            return this->NoFinishAndAssignType(CommandTypes::ARRAY_DECLARATION_START);
        }
        else
        {
            return this->FinishAndAssignType(CommandTypes::VAR_DECLARATION);
        }
    }
    else if (this->IsType(nextToken) && 
             this->IsValue(currToken, "unsigned") && 
             !this->IsValue(currToken, "double"))
    {
        return false;
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseParameterKeywordCall(Token& currToken)
{
    if (this->IsValue(currToken, "("))
    {
        this->HandleFoundParantheses();
        this->m_bFirstParanthesesEnter = false;

        if (this->m_sType == CommandTypes::ELIF_CALL)
        {
            return this->NoFinishAndAssignType(CommandTypes::ELIF_START);
        }
        else if (this->m_sType == CommandTypes::IF_CALL)
        {
            return this->NoFinishAndAssignType(CommandTypes::IF_START);
        }
        else if (this->m_sType == CommandTypes::FOR_CALL)
        {
            return this->NoFinishAndAssignType(CommandTypes::FOR_START);
        }
        else if (this->m_sType == CommandTypes::WHILE_CALL)
        {
            return this->NoFinishAndAssignType(CommandTypes::WHILE_START);
        }
        else if (this->m_sType == CommandTypes::SIZEOF_CALL)
        {
            return this->NoFinishAndAssignType(CommandTypes::SIZEOF_START);
        }
        else
        {
            return this->FoundUnknown(currToken);
        }
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseParameterKeywordStart(Token& currToken, Token& nextToken)
{
    if (this->HandleDefaultPossibleParamChecks(currToken, "("))
    {
        return false;
    }
    else if (this->IsValue(currToken, ")")) 
    {
        if (this->LowerParanthesesCounter(currToken))
        {
            return true;
        }
        if (this->m_iParanthesesCounter == 0)
        {
            this->m_bBuildingParam = false;
            this->m_bIgnoreCurrToken = false;

            if (this->m_sType == CommandTypes::ELIF_START)
            {
                return this->FinishAndAssignType(CommandTypes::ELIF);
            }
            else if (this->m_sType == CommandTypes::IF_START)
            {
                return this->FinishAndAssignType(CommandTypes::IF);
            }
            else if (this->m_sType == CommandTypes::FOR_START)
            {
                return this->FinishAndAssignType(CommandTypes::FOR);
            }
            else if (this->m_sType == CommandTypes::WHILE_START)
            {
                return this->FinishAndAssignType(CommandTypes::WHILE);
            }
            else if (this->m_sType == CommandTypes::SIZEOF_START)
            {
                return this->FinishAndAssignType(CommandTypes::SIZEOF);
            }
            else
            {
                return this->FoundUnknown(currToken);
            }
        }
        this->m_bIgnoreCurrToken = true;
    }

    this->HandleKeywordParams(currToken, nextToken);
    return false;
}

bool Command::ParseExpressionValue(Token& currToken)
{
    if (this->IsOperator(currToken))
    {
        return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
    }
    else if (this->IsValue(currToken, ")"))
    {
        this->LowerParanthesesCounter(currToken);
        return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseExpressionOperator(Token& currToken, Token& nextToken)
{
    if (this->m_bBuildingParam)
    {
        this->m_bIgnoreCurrToken = true;

        if (this->IsValue(currToken, "("))
        {
            this->m_iParanthesesCounter++;
        }
        else if (this->IsValue(currToken, ")"))
        {
            this->m_iParanthesesCounter--;
        }

        if (this->m_iParanthesesCounter == 0)
        {
            if (this->AddToCurrParam(currToken, currToken)) 
            {
                this->m_bBuildingParam = false;
                return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
            }
            return false;
        }
        else if (this->AddToCurrParam(currToken, nextToken)) 
        {
            this->m_bBuildingParam = false;
            return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else
        {
            return false;
        }
    }
    else if (this->IsValueToken(currToken))
    {
        if (this->IsType(currToken, TokenTypes::IDENTIFIER)) 
        {
            if (this->IsValue(nextToken, "(") || this->IsValue(nextToken, "["))
            {
                this->m_bIgnoreCurrToken = true;
                this->m_bBuildingParam = true;
                this->AddToCurrParam(currToken, nextToken);
                return false;
            }
            else 
            {
                this->AddIdentifier(currToken);
                return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
            }
        } 
        else if (this->IsType(currToken, TokenTypes::VALUE) || 
                 this->IsType(currToken, TokenTypes::STRING_LITERAL))
        {
            return this->NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else
        {
            return this->FoundUnknown(currToken);
        }
    }
    else if (this->IsValue(currToken, "("))
    {
        this->m_iParanthesesCounter++;
        return false;
    }
    else if (this->IsValue(currToken, ")"))
    {
        return this->LowerParanthesesCounter(currToken);
    }
    else if (this->IsValue(currToken, "not"))
    {
        return false;
    }
    else if (this->IsValue(currToken, "sizeof"))
    {
        this->m_bBuildingParam = true;
        this->AddToCurrParam(currToken, nextToken);
        return false;
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseNotFuncIdentifier(Token& currToken)
{
    if (currToken.GetValue() == ":")
    {
        return this->NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    }
    else
    {
        return this->FoundUnknown(currToken);
    }
}

bool Command::ParseArrayAccessStart(Token& currToken, Token& nextToken)
{
    if (this->HandleDefaultPossibleParamChecks(currToken, "["))
    {
        return false;
    }
    else if (currToken.GetValue() == "]")
    {
        if (this->LowerParanthesesCounter(currToken))
        {
            return true;
        }
        else if (this->m_iParanthesesCounter == 0)
        {
            this->m_bIgnoreCurrToken = false;
            return this->FinishAndAssignType(CommandTypes::ARRAY_ACCESS);
        }
    }

    this->AddToCurrParam(currToken, nextToken);
    return false;
}

bool Command::ParseArrayDeclarationStart(Token& currToken, Token& nextToken)
{
    return this->HandleArrayParanthesesParams(
        currToken, nextToken, CommandTypes::ARRAY_DECLARATION);
}

bool Command::ParseArrayValueStart(Token& currToken, Token& nextToken)
{
    return this->HandleArrayParanthesesParams(
        currToken, nextToken, CommandTypes::ARRAY_VALUE);
}


inline bool Command::IsValue(Token& token, std::string sCompare)
{
    return token.GetValue() == sCompare;
}

inline bool Command::IsType(Token& token, std::string_view sCompare)
{
    return token.GetType() == sCompare;
}

bool Command::CompareExpected(
    Token& token, std::string sCompare, std::string_view sValidType)
{
    if (this->IsValue(token, sCompare))
    {
        this->m_sType = sValidType;
        return false;
    }
    else
    {
        return this->FoundUnknown(token);
    }
}

bool Command::IsAccessRights(Token& token)
{
    return this->IsValue(token, "const") || this->IsValue(token, "readonly");
}

bool Command::IsScope(Token& token)
{
    return this->IsValue(token, "global") ||
           this->IsValue(token, "var")    || 
           this->IsValue(token, "let");
}

bool Command::IsExpressionStart(Token& token)
{
    return this->IsType(token, TokenTypes::IDENTIFIER)     ||
           this->IsType(token, TokenTypes::VALUE)          ||
           this->IsType(token, TokenTypes::STRING_LITERAL) ||
           this->IsValue(token, "-")                       ||
           this->IsValue(token, "not")                     ||
           this->IsValue(token, "(");
}

inline bool Command::IsOperator(Token& token)
{
    return (this->IsType(token, TokenTypes::OPERATOR) && !this->IsValue(token, "=")) ||
           this->IsValue(token, "and") ||
           this->IsValue(token, "or")  ||
           this->IsValue(token, "xor");
}

inline bool Command::IsValueToken(Token& token)
{
    return this->IsType(token, TokenTypes::IDENTIFIER)     ||
           this->IsType(token, TokenTypes::VALUE)          ||
           this->IsType(token, TokenTypes::STRING_LITERAL);
}

inline bool Command::IsType(Token& token)
{
    return this->IsValue(token, "int")    ||
           this->IsValue(token, "bool")   ||
           this->IsValue(token, "double") ||
           this->IsValue(token, "void");
}

bool Command::IsControlStatement(Token& token)
{
    return this->IsValue(token, "break") || this->IsValue(token, "continue");
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

inline bool Command::FoundUnknown(Token& currToken)
{
    if (this->m_errors.size() == 0)
    {
        this->m_errors.push_back(
            CompileTimeError(ErrorTypes::SYNTAX_ERROR, 
            "Illegal command found",
            currToken.GetLine(), currToken.GetColumn(),
            ""));
    }
    this->m_sType = CommandTypes::UNKNOWN;
    return true;
}


inline bool Command::AddToCurrParam(Token& currToken, Token& nextToken)
{
    if (this->m_uParameterIndex == this->m_parameters.size()) 
    {
        this->m_parameters.push_back(Command());
        this->m_tokens.push_back(Token(
            TokenTypes::HAS_PARAM,
            currToken.GetLine(), currToken.GetColumn()));
    }
    if (this->m_parameters[this->m_uParameterIndex].Append(currToken, nextToken))
    {
        this->m_uParameterIndex++;
        return true;
    }
    return false;
}

bool Command::LowerParanthesesCounter(Token& currToken)
{
    this->m_iParanthesesCounter--;
    if (this->m_iParanthesesCounter < 0)
    {
        this->m_errors.push_back(CompileTimeError(
            ErrorTypes::SYNTAX_ERROR,
            "Unexpected ')' — too many closing parentheses",
            currToken.GetLine(), currToken.GetColumn(),
            "Check for a missing opening '('"));
        return this->FoundUnknown(currToken);
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

void Command::AddIdentifier(Token& currToken)
{
    this->m_bIgnoreCurrToken = true;
    this->m_tokens.push_back(Token(
        TokenTypes::HAS_PARAM,
        currToken.GetLine(), currToken.GetColumn()));
    this->m_parameters.push_back(Command());
    this->m_parameters[this->m_uParameterIndex].m_tokens.push_back(currToken);
    this->m_parameters[this->m_uParameterIndex].m_sType = CommandTypes::IDENTIFIER_VALUE;
    this->m_bIgnoreCurrToken = true;
    this->m_uParameterIndex++;
}


bool Command::HandleFuncLikeParams(Token& currToken, Token& nextToken)
{
    if (this->IsValue(currToken, ",")) 
    {
        this->m_tokens.push_back(currToken);
        if (this->m_uParameterIndex < this->m_parameters.size())
        {
            this->m_uParameterIndex++;
        }
        return false;
    }
    else
    {
        return this->AddToCurrParam(currToken, nextToken);
    }
}

bool Command::HandleKeywordParams(Token& currToken, Token& nextToken)
{
    if (this->IsValue(nextToken, ";"))
    {
        this->m_bIgnoreEON = true;
    }
    if (this->IsValue(currToken, ";")) 
    {
        this->m_tokens.push_back(currToken);
        if (this->m_uParameterIndex < this->m_parameters.size())
        {
            this->m_uParameterIndex++;
        }
        return false;
    }
    else if (this->m_tokens.size() == 2 && this->IsType(currToken) && 
             this->m_sType == CommandTypes::SIZEOF_START)
    {
        this->m_tokens.push_back(Token(
            TokenTypes::HAS_PARAM,
            currToken.GetLine(), currToken.GetColumn()));
        this->m_parameters.push_back(Command());
        this->m_parameters[this->m_uParameterIndex].m_tokens.push_back(currToken);
        this->m_parameters[this->m_uParameterIndex].m_sType = CommandTypes::VAR_TYPE;
        this->m_uParameterIndex++;
        return false;
    }
    else
    {
        return this->AddToCurrParam(currToken, nextToken);
    }
}

inline bool Command::HandleFoundParantheses()
{
    this->m_bBuildingParam = true;
    this->m_bFirstParanthesesEnter = true;
    this->m_iParanthesesCounter++;
    return false;
}

bool Command::HandleDefaultPossibleParamChecks(
    Token& currToken, std::string sParantheses)
{
    if (this->IsValue(currToken, sParantheses)) 
    {
        if (this->m_bFirstParanthesesEnter)
        {
            this->m_bFirstParanthesesEnter = !this->m_bFirstParanthesesEnter;
            return true;
        }
        else
        {
            this->m_iParanthesesCounter++;
        }
    }
    this->m_bIgnoreCurrToken = true;
    return false;
}

bool Command::HandleArrayParanthesesParams(
    Token& currToken, Token& nextToken, std::string_view sFinishedType)
{
    if (this->HandleDefaultPossibleParamChecks(currToken, "["))
    {
        return false;
    }
    else if (currToken.GetValue() == "]")
    {
        if (this->LowerParanthesesCounter(currToken))
        {
            return true;
        }
        if (this->m_iParanthesesCounter == 0)
        {
            this->m_bIgnoreCurrToken = false;
            return this->FinishAndAssignType(sFinishedType);
        }
    }

    this->HandleFuncLikeParams(currToken, nextToken);
    return false;
}
