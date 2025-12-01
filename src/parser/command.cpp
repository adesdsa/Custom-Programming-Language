#include "includes/parser/command.h"


Command::Command(): m_sType(CommandTypes::UNIDENTIFIED), m_uParameterIndex(0),
 m_bBuildingParam(false), m_bIgnoreCurrToken(false), m_bIgnoreEON(false),
 m_iParanthesesCounter(0), m_bFirstParanthesesEnter(false)
{
}


std::string_view Command::GetType() const {
    return m_sType;
}

const std::vector<Token>& Command::GetTokens() const {
    return m_tokens;
}

bool Command::HasParams() const noexcept
{
    return this->m_parameters.size() > 0;
}

std::vector<Command>& Command::GetParams() {
    return m_parameters;
}

const std::vector<Command>& Command::GetParams() const {
    return m_parameters;
}

unsigned int Command::GetParanthesesCounter() const {
    return m_iParanthesesCounter;
}

bool Command::IsExpression() const {
    return m_bExpression;
}

const std::vector<CompileTimeError>& Command::GetErrors() const {
    return m_errors;
}

void Command::SetType(std::string_view type) {
    m_sType = type;
}

void Command::SetExpression(bool indicator) {
    m_bExpression = indicator;
}

Token& Command::operator[](size_t index) {
    return m_tokens[index];
}

const Token& Command::operator[](size_t index) const {
    return m_tokens[index];
}


bool Command::Append(Token& currToken, Token& nextToken)
{
    m_bIgnoreCurrToken = false; // reset
    m_bIgnoreEON = false;       // reset
    bool bReturnValue = false;

    if (m_sType == CommandTypes::UNIDENTIFIED) bReturnValue = 
        ParseUnidentified(currToken, nextToken);
    else if (m_sType == CommandTypes::ACCESS_RIGHTS) bReturnValue = 
        ParseAccessRights(currToken, nextToken);
    else if (m_sType == CommandTypes::SCOPE) bReturnValue = 
        ParseScope(currToken, nextToken);
    else if (m_sType == CommandTypes::IDENTIFIER) bReturnValue = 
        ParseIdentifier(currToken, nextToken);
    else if (m_sType == CommandTypes::FUNC_START) bReturnValue = 
        ParseFuncStart(currToken, nextToken);
    else if (m_sType == CommandTypes::FILLED_FUNC_DECLARATION) bReturnValue = 
        ParseFilledFuncDeclaration(currToken, nextToken);
    else if (m_sType == CommandTypes::ARROW_FUNC_DECLARATION) bReturnValue = 
        ParseArrowFuncDeclaration(currToken, nextToken);
    else if (m_sType == CommandTypes::VAR_DECLARATION_START) bReturnValue = 
        ParseVarDeclarationStart(currToken, nextToken);
    else if (m_sType == CommandTypes::ELIF_CALL || 
        m_sType == CommandTypes::IF_CALL || m_sType == CommandTypes::FOR_CALL
        || m_sType == CommandTypes::WHILE_CALL ||
        m_sType == CommandTypes::SIZEOF_CALL) 
        bReturnValue = ParseParameterKeywordCall(currToken, nextToken);
    else if (m_sType == CommandTypes::ELIF_START ||
        m_sType == CommandTypes::IF_START
        || m_sType == CommandTypes::FOR_START
        || m_sType == CommandTypes::WHILE_START
        || m_sType == CommandTypes::SIZEOF_START) 
        bReturnValue = ParseParameterKeywordStart(currToken, nextToken);
    else if (m_sType == CommandTypes::EXPRESSION_VALUE) bReturnValue = 
        ParseExpressionValue(currToken, nextToken);
    else if (m_sType == CommandTypes::EXPRESSION_OPERATOR) bReturnValue = 
        ParseExpressionOperator(currToken, nextToken);
    else if (m_sType == CommandTypes::POINTER_DECLARATION) bReturnValue = 
        ParsePointerDeclaration(currToken, nextToken);
    else if (m_sType == CommandTypes::NOT_FUNC_IDENTIFIER) bReturnValue = 
        ParseNotFuncIdentifier(currToken, nextToken);
    else if (m_sType == CommandTypes::ARRAY_ACCESS_START) bReturnValue = 
        ParseArrayAccessStart(currToken, nextToken);
    else if (m_sType == CommandTypes::ARRAY_DECLARATION_START) bReturnValue = 
        ParseArrayDeclarationStart(currToken, nextToken);
    else if (m_sType == CommandTypes::ARRAY_VALUE_START) bReturnValue = 
        ParseArrayValueStart(currToken, nextToken);
    else return FoundUnknown(currToken);


    if (!m_bIgnoreCurrToken) 
        m_tokens.push_back(currToken);
    if (IsValue(nextToken, ";") && !m_bIgnoreEON) 
        bReturnValue = true;
    if (IsValue(nextToken, "{") || IsValue(nextToken, "}")) 
        bReturnValue = true;

    return bReturnValue;

    // extra rules: todo
    // there is no support for int a, b, c=0;
    // & * (in the context of pointers) should appear before the identifier when they are called
    // When declaring a pointer the * should appear after the type 
    // no support for array pointers/2d arrays
    // right now we have infinite func params but no default values
    // right now single line keyword statements do not support single lines but only code blocks.
    // In PicoC giving a value to an array will be [1 ,2 ,3] and not {1, 2, 3}
    // Note that if code can be outside of {} is checked in the semantic analysis and not in the parser
    // A for loop must be of the next prset: for ( var_declaration/identifier assigmnet value ; expression ; expression )
    // scope and access_rights arent a must have.
}

bool Command::ParseUnidentified(Token& currToken, Token& nextToken)
{
    if (IsType(currToken, TokenTypes::IDENTIFIER) && IsValue(nextToken, "="))
        return FinishAndAssignType(CommandTypes::IDENTIFIER);
    else if (IsType(currToken, TokenTypes::IDENTIFIER))
    {
        if (IsOperator(nextToken))
        {
            m_tokens.push_back(Token(TokenTypes::HAS_PARAM, 
                currToken.GetLine(), currToken.GetColumn()));
            m_parameters.push_back(Command());
            m_parameters[0].m_tokens.push_back(currToken);
            m_parameters[0].m_sType = CommandTypes::IDENTIFIER;
            m_bIgnoreCurrToken = true;
            m_uParameterIndex++;
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else return NoFinishAndAssignType(CommandTypes::IDENTIFIER);
    }
    else if (IsType(currToken, TokenTypes::KEYWORD))
    {
        if (IsValue(currToken, "void"))
            return FinishAndAssignType(m_sType = CommandTypes::VOID_COMMAND);
        else if (IsAccessRights(currToken))
            return NoFinishAndAssignType(CommandTypes::ACCESS_RIGHTS);
        else if (IsScope(currToken))
            return NoFinishAndAssignType(m_sType = CommandTypes::SCOPE);
        else if (IsValue(currToken, "else"))
            return FinishAndAssignType(m_sType = CommandTypes::ELSE);
        else if (IsValue(currToken, "if"))
            return CompareExpected(nextToken, "(", CommandTypes::IF_CALL);
        else if (IsValue(currToken, "elif"))
            return CompareExpected(nextToken, "(", CommandTypes::ELIF_CALL);
        else if (IsValue(currToken, "for"))
            return CompareExpected(nextToken, "(", CommandTypes::FOR_CALL);
        else if (IsValue(currToken, "while"))
            return CompareExpected(nextToken, "(", CommandTypes::WHILE_CALL);
        else if (IsValue(currToken, "sizeof"))
            return CompareExpected(nextToken, "(", CommandTypes::SIZEOF_CALL);
        else if (IsValue(currToken, "return"))
            return FinishAndAssignType(CommandTypes::RETURN);
        else if (IsControlStatement(currToken))
            return FinishAndAssignType(CommandTypes::CONTROL_STATEMENT);
        else if (IsValue(currToken, "pass"))
            return FinishAndAssignType(CommandTypes::PASS);
        else return FoundUnknown(currToken);
    }
    else if (IsExpressionStart(currToken))
    {
        if (IsValue(currToken, "-") || IsValue(currToken, "not") ||
            IsValue(currToken, "*") || IsValue(currToken, "&") || 
            IsValue(currToken, "("))
        {
            if (IsValue(currToken, "("))
            {
                m_iParanthesesCounter++;
            }
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
        }
        else return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
    }
    else if (IsType(currToken, TokenTypes::PUNCTUATION))
    {
        if (IsValue(currToken, ";"))
            return FinishAndAssignType(CommandTypes::END_OF_COMMAND);
        else if (IsValue(currToken, "{"))
            return FinishAndAssignType(CommandTypes::BLOCK_START);
        else if (IsValue(currToken, "}"))
            return FinishAndAssignType(CommandTypes::BLOCK_END);
        else if (IsValue(currToken, "["))
        {
            HandleFoundParantheses();
            return NoFinishAndAssignType(CommandTypes::ARRAY_VALUE_START);
        }
        else return FoundUnknown(currToken);
    }
    else if (IsValue(currToken, "="))
        return FinishAndAssignType(CommandTypes::ASSIGNMENT);
    else if (IsType(currToken, TokenTypes::EOF_TOKEN))
        return FinishAndAssignType(CommandTypes::EOF_TOKEN);
    else return FoundUnknown(currToken);
}

bool Command::ParseIdentifier(Token& currToken, Token& nextToken)
{
    if (IsValue(currToken, "(") || IsValue(currToken, "[")) 
    {
        HandleFoundParantheses();
        m_bFirstParanthesesEnter = false;
        if (IsValue(currToken, "("))
            return NoFinishAndAssignType(CommandTypes::FUNC_START);
        else return NoFinishAndAssignType(CommandTypes::ARRAY_ACCESS_START);
    }
    else if (currToken.GetValue() == ":")
        return NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    else return FoundUnknown(currToken);
}

bool Command::ParseFuncStart(Token& currToken, Token& nextToken)
{
    if (HandleDefaultPossibleParamChecks(currToken, "(")) return false;
    else if (currToken.GetValue() == ")")
    {
        if(LowerParanthesesCounter(currToken)) return true;
        else if (m_iParanthesesCounter == 0)
        {
            m_bIgnoreCurrToken = false;
            if (IsValue(nextToken, "->"))
                return NoFinishAndAssignType(CommandTypes::FILLED_FUNC_DECLARATION);
            else if (IsOperator(nextToken))
            {
                m_bIgnoreCurrToken = true;
                m_tokens.push_back(currToken);
                m_sType = CommandTypes::FINISHED_FUNC_CALL;
                MoveThisIntoParam();
                return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
            }
            else return FinishAndAssignType(CommandTypes::FINISHED_FUNC_CALL);
        }
    }

    HandleFuncLikeParams(currToken, nextToken);
    return false;
}

bool Command::ParseFilledFuncDeclaration(Token& currToken, Token& nextToken)
{
    if (IsValue(currToken, "->"))
        return NoFinishAndAssignType(CommandTypes::ARROW_FUNC_DECLARATION);
    else return FoundUnknown(currToken);
}

bool Command::ParseArrowFuncDeclaration(Token& currToken, Token& nextToken)
{
    if (IsType(currToken))
        return FinishAndAssignType(CommandTypes::FINISHED_FUNC_DECLARATION);
    else if (IsValue(currToken, "unsigned") && IsType(nextToken)) return false;
    else return FoundUnknown(currToken);
}

bool Command::ParseAccessRights(Token& currToken, Token& nextToken)
{
    if (IsScope(currToken)) return NoFinishAndAssignType(CommandTypes::SCOPE);
    else if (IsType(currToken, TokenTypes::IDENTIFIER))
        return NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    else return FoundUnknown(currToken);
}

bool Command::ParseScope(Token& currToken, Token& nextToken)
{
    if (IsType(currToken, TokenTypes::IDENTIFIER))
        return NoFinishAndAssignType(CommandTypes::NOT_FUNC_IDENTIFIER);
    else return FoundUnknown(currToken);
}

bool Command::ParseVarDeclarationStart(Token& currToken, Token& nextToken)
{
    if (IsType(currToken))
    {
        if (IsValue(nextToken, "[")) 
        {
            HandleFoundParantheses();
            return NoFinishAndAssignType(
                CommandTypes::ARRAY_DECLARATION_START);
        }
        if (IsValue(nextToken, "*"))
            return NoFinishAndAssignType(CommandTypes::POINTER_DECLARATION);
        else return FinishAndAssignType(CommandTypes::VAR_DECLARATION);
    }
    else return FoundUnknown(currToken);
}

bool Command::ParseParameterKeywordCall(Token& currToken, Token& nextToken)
{
    if (IsValue(currToken, "("))
    {
        HandleFoundParantheses();
        m_bFirstParanthesesEnter = false;
        if (m_sType == CommandTypes::ELIF_CALL)
            return NoFinishAndAssignType(CommandTypes::ELIF_START);
        else if (m_sType == CommandTypes::IF_CALL)
            return NoFinishAndAssignType(CommandTypes::IF_START);
        else if (m_sType == CommandTypes::FOR_CALL)
            return NoFinishAndAssignType(CommandTypes::FOR_START);
        else if (m_sType == CommandTypes::WHILE_CALL)
            return NoFinishAndAssignType(CommandTypes::WHILE_START);
        else if (m_sType == CommandTypes::SIZEOF_CALL)
            return NoFinishAndAssignType(CommandTypes::SIZEOF_START);
        else return FoundUnknown(currToken);
    }
    else return FoundUnknown(currToken);
}

bool Command::ParseParameterKeywordStart(Token& currToken, Token& nextToken)
{
    if (HandleDefaultPossibleParamChecks(currToken, "(")) return false;
    else if (IsValue(currToken, ")")) 
    {
        if(LowerParanthesesCounter(currToken)) return true;
        if (m_iParanthesesCounter == 0)
        {
            m_bBuildingParam = false;
            m_bIgnoreCurrToken = false;
            if (m_sType == CommandTypes::ELIF_START)
                return FinishAndAssignType(CommandTypes::ELIF);
            else if (m_sType == CommandTypes::IF_START)
                return FinishAndAssignType(CommandTypes::IF);
            else if (m_sType == CommandTypes::FOR_START)
                return FinishAndAssignType(CommandTypes::FOR);
            else if (m_sType == CommandTypes::WHILE_START)
                return FinishAndAssignType(CommandTypes::WHILE);
            else if (m_sType == CommandTypes::SIZEOF_START)
                return FinishAndAssignType(CommandTypes::SIZEOF);
            else return FoundUnknown(currToken);
        }
    }

    HandleKeywordParams(currToken, nextToken);
    return false;
}

bool Command::ParseExpressionValue(Token& currToken, Token& nextToken)
{
    if (IsOperator(currToken)) return NoFinishAndAssignType(
        CommandTypes::EXPRESSION_OPERATOR);
    else if (IsValue(currToken, ")"))
    {
        LowerParanthesesCounter(currToken);
        return NoFinishAndAssignType(CommandTypes::EXPRESSION_OPERATOR);
    }
    else return FoundUnknown(currToken);
}

bool Command::ParseExpressionOperator(Token& currToken, Token& nextToken)
{
    if (m_bBuildingParam)
    {
        m_bIgnoreCurrToken = true;
        if (AddToCurrParam(currToken, nextToken)) 
        {
            m_bBuildingParam = false;
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        }
        else return false;
    }
    else if (IsValueToken(currToken))
    {
        if (IsType(currToken, TokenTypes::IDENTIFIER)) {
            if (IsValue(nextToken, "(") || IsValue(nextToken, "["))
            {
                m_bIgnoreCurrToken = true;
                m_bBuildingParam = true;
                AddToCurrParam(currToken, nextToken);
                return false;
            }
            else 
            {
                AddIdentifier(currToken);
                return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
            }
        } 
        else if (IsType(currToken, TokenTypes::VALUE)) 
            return NoFinishAndAssignType(CommandTypes::EXPRESSION_VALUE);
        else return FoundUnknown(currToken);
    }
    else if (IsValue(currToken, "("))
    {
        m_iParanthesesCounter++;
        return false;
    }
    else if (IsValue(currToken, ")")) return LowerParanthesesCounter(currToken);
    else if (IsValue(currToken, "not") || IsValue(currToken, "*") ||  
        IsValue(currToken, "&")) return false;
    else if (IsValue(currToken, "sizeof"))
    {
        m_bBuildingParam = true;
        AddToCurrParam(currToken, nextToken);
        return false;
    }
    else return FoundUnknown(currToken);
}

inline bool Command::ParsePointerDeclaration(
    Token& currToken, Token& nextToken)
{
    return FinishAndAssignType(CommandTypes::POINTER_DECLARATION);
}

bool Command::ParseNotFuncIdentifier(Token& currToken, Token& nextToken)
{
    if (currToken.GetValue() == ":")
        return NoFinishAndAssignType(CommandTypes::VAR_DECLARATION_START);
    else return FoundUnknown(currToken);
}

bool Command::ParseArrayAccessStart(Token& currToken, Token& nextToken)
{
    if (HandleDefaultPossibleParamChecks(currToken, "[")) return false;
    else if (currToken.GetValue() == "]")
    {
        if (LowerParanthesesCounter(currToken)) return true;
        else if (m_iParanthesesCounter == 0)
        {
            m_bIgnoreCurrToken = false;
            return FinishAndAssignType(CommandTypes::ARRAY_ACCESS);
        }
    }

    AddToCurrParam(currToken, nextToken);
    return false;
}

bool Command::ParseArrayDeclarationStart(Token& currToken, Token& nextToken)
{
    return HandleArrayParanthesesParams(
        currToken, nextToken, CommandTypes::ARRAY_DECLARATION);
}

bool Command::ParseArrayValueStart(Token& currToken, Token& nextToken)
{
    return HandleArrayParanthesesParams(
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
    if (IsValue(token, sCompare))
    {
        m_sType = sValidType;
        return false;
    }
    else return FoundUnknown(token);
}

bool Command::IsAccessRights(Token& token)
{
    return IsValue(token, "const") || IsValue(token, "readonly");
}

bool Command::IsScope(Token& token)
{
    return IsValue(token, "global") ||
        IsValue(token, "var") || IsValue(token, "let");
}

bool Command::IsExpressionStart(Token& token)
{
    return IsType(token, TokenTypes::IDENTIFIER) ||
        IsType(token, TokenTypes::VALUE) ||
        IsValue(token, "-") ||
        IsValue(token, "not") ||
        IsValue(token, "*") ||
        IsValue(token, "&") ||
        IsValue(token, "(");
}

inline bool Command::IsOperator(Token& token)
{
    return (IsType(token, TokenTypes::OPERATOR) && !(
        IsValue(token, "=") ||
        IsValue(token, "&"))) ||
        IsValue(token, "and") ||
        IsValue(token, "or") ||
        IsValue(token, "xor") ||
        IsValue(token, "is");
}

inline bool Command::IsValueToken(Token& token)
{
    return IsType(token, TokenTypes::IDENTIFIER) ||
        IsType(token, TokenTypes::VALUE);
}

inline bool Command::IsType(Token& token)
{
    return IsValue(token, "int") ||
    IsValue(token, "bool") ||
    IsValue(token, "double") ||
    IsValue(token, "void");
}

bool Command::IsControlStatement(Token& token)
{
    return IsValue(token, "break") || IsValue(token, "continue");
}


inline bool Command::FinishAndAssignType(std::string_view sType)
{
    m_sType = sType;
    return true;
}

inline bool Command::NoFinishAndAssignType(std::string_view sType)
{
    m_sType = sType;
    return false;
}

inline bool Command::FoundUnknown(Token& currToken)
{
    if (m_errors.size() == 0) m_errors.push_back(
        CompileTimeError(ErrorTypes::SYNTAX_ERROR, 
        "Illegal command found",
        currToken.GetLine(), currToken.GetColumn(),
        ""));
    m_sType = CommandTypes::UNKNOWN;
    return true;
}


inline bool Command::AddToCurrParam(Token& currToken, Token& nextToken)
{
    if (m_uParameterIndex == m_parameters.size()) 
    {
        m_parameters.push_back(Command());
        m_tokens.push_back(Token(
            TokenTypes::HAS_PARAM,
            currToken.GetLine(), currToken.GetColumn()));
    }
    if (m_parameters[m_uParameterIndex].Append(currToken, nextToken))
    {
        m_uParameterIndex++;
        return true;
    }
    return false;
}

bool Command::LowerParanthesesCounter(Token& currToken)
{
    m_iParanthesesCounter--;
    if (m_iParanthesesCounter < 0)
    {
        m_errors.push_back(CompileTimeError(
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

    m_bBuildingParam = false;
    m_bIgnoreEON = false;
    m_bExpression = false;
    m_iParanthesesCounter = 0;
    m_uParameterIndex = 1;
    m_tokens.push_back(Token(
        TokenTypes::HAS_PARAM,
        tempSave.m_tokens[0].GetLine(), tempSave.m_tokens[0].GetColumn()));
    m_parameters.push_back(tempSave);
}

void Command::AddIdentifier(Token& currToken)
{
    m_bIgnoreCurrToken = true;
    m_tokens.push_back(Token(
        TokenTypes::HAS_PARAM,
        currToken.GetLine(), currToken.GetColumn()));
    m_parameters.push_back(Command());
    m_parameters[m_uParameterIndex].m_tokens.push_back(currToken);
    m_parameters[m_uParameterIndex].m_sType = CommandTypes::IDENTIFIER;
    m_bIgnoreCurrToken = true;
    m_uParameterIndex++;
}


bool Command::HandleFuncLikeParams(Token& currToken, Token& nextToken)
{
    if (IsValue(currToken, ",")) 
    {
        m_tokens.push_back(currToken);
        if (m_uParameterIndex < m_parameters.size())
            m_uParameterIndex++;
        return false;
    }
    else return AddToCurrParam(currToken, nextToken);
}

bool Command::HandleKeywordParams(Token& currToken, Token& nextToken)
{
    if (IsValue(nextToken, ";")) m_bIgnoreEON = true;
    if (IsValue(currToken, ";")) 
    {
        m_tokens.push_back(currToken);
        if (m_uParameterIndex < m_parameters.size())
            m_uParameterIndex++;
        return false;
    }
    else if (IsType(currToken) && m_sType == CommandTypes::SIZEOF_START)
    {
        m_tokens.push_back(Token(
            TokenTypes::HAS_PARAM,
            currToken.GetLine(), currToken.GetColumn()));
        m_parameters.push_back(Command());
        m_parameters[m_uParameterIndex].m_tokens.push_back(currToken);
        m_parameters[m_uParameterIndex].m_sType = CommandTypes::VAR_TYPE;
        m_uParameterIndex++;
    }
    else return AddToCurrParam(currToken, nextToken);
}

inline bool Command::HandleFoundParantheses()
{
    m_bBuildingParam = true;
    m_bFirstParanthesesEnter = true;
    m_iParanthesesCounter++;
    return false;
}

bool Command::HandleDefaultPossibleParamChecks(
    Token& currToken, std::string sParantheses)
{
    if (IsValue(currToken, sParantheses)) 
    {
        if (m_bFirstParanthesesEnter)
        {
            m_bFirstParanthesesEnter = !m_bFirstParanthesesEnter;
            return true;
        }
        else m_iParanthesesCounter++;
    }
    m_bIgnoreCurrToken = true;
    return false;
}

bool Command::HandleArrayParanthesesParams(
    Token& currToken, Token& nextToken, std::string_view sFinishedType)
{
    if (HandleDefaultPossibleParamChecks(currToken, "[")) return false;
    else if (currToken.GetValue() == "]")
    {
        if (LowerParanthesesCounter(currToken)) return true;
        if (m_iParanthesesCounter == 0)
        {
            m_bIgnoreCurrToken = false;
            return FinishAndAssignType(sFinishedType);
        }
    }

    HandleFuncLikeParams(currToken, nextToken);
    return false;
}
