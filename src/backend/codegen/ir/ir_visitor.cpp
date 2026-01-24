#include "includes/backend/codegen/ir/ir_visitor.h"

IRVisitor::IRVisitor() {}

void IRVisitor::VisitVarDeclaration(ASTNode* node)
{
    std::string varName = ASTExtractor::ExtractVarName(node);
    const VarMetaData& varMetaData = m_decoratedAst.varTable.FindVar(varName);
    if (!varMetaData) return;
    m_currVarMetaData = varMetaData;

    if (varMetaData.scope == Scope::GLOBAL)
    {
        m_currModule.AddVariable
        (varMetaData.sName,
            TypeToIRType(
                varMetaData.type
            )
        );
    }
    else
    {
        m_irbuilder.CreateAlloca(
            TypeToIRType(
                varMetaData.type
            ),
            varMetaData.sName
        );
    }
}

void IRVisitor::VisitExpression(ASTNode* node)
{
    IRBaseValue* expressionValue = HandleExpression(node->command);

    CheckValueNeededSomewhere(expressionValue, node);

    this->m_irbuilder.ClearConstants(); /// @todo handle this
}

void IRVisitor::VisitIdentifier(ASTNode* node)
{
    std::string varName = ASTExtractor::ExtractVarName(node);
    const VarMetaData& varMetaData = m_decoratedAst.varTable.FindVar(varName);
    if (!varMetaData) return;
    m_currVarMetaData = varMetaData;
}

void IRVisitor::VisitIdentifierValue(ASTNode* node)
{
    (void)HandleVariableValue(node->command);
}

void IRVisitor::VisitAssignment(ASTNode* node)
{
    /// @todo implement
    if (this->m_expectedAssignment)
    {
        this->m_irbuilder.CreateStore(
            this->m_lastSavedValue,
            IRBaseValue(
                varMetaData.sName,
                TypeToIRType(
                    varMetaData.type
                )
            )
        );
        this->m_expectedAssignment = false;
    }
    else
    {
        this->m_expectedAssignment = true;
    }
}

void IRVisitor::VisitFuncDeclaration(ASTNode* node)
{
    const FuncMetaData& funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(innerCommand[0]);
    if (!funcMetaData) return;
    this->m_currFunc = this->m_currModule.AddFunction(funcMetaData.GetName(), funcMetaData.GetRType());
    for (auto argMetaData : funcMetaData.GetFuncParams())
    {
        this->m_currFunc.AddArgument(argMetaData.sName, TypeToIRType(argMetaData.type));
    }
    m_blockHistory.push(BlockKind::functionBlock);
}

void IRVisitor::VisitFuncCall(ASTNode* node)
{
    /// @todo check if the function is already declared if not first call visitdeclaration
    (void)HandleFuncCall(node->command);
}

void IRVisitor::VisitIf(ASTNode* node)
{
    /// @todo phi handling + create IRBlock
    m_blockHistory.push(BlockKind::ifBlock);
}

void IRVisitor::VisitElif(ASTNode* node)
{
    /// @todo phi handling + create IRBlock
    m_blockHistory.push(BlockKind::elifBlock);
}

void IRVisitor::VisitElse(ASTNode* node)
{
    /// @todo phi handling + create IRBlock
    m_blockHistory.push(BlockKind::elseBlock);
}

void IRVisitor::VisitWhile(ASTNode* node)
{
    /// @todo search how to handle then implement + create IRBlock
    m_blockHistory.push(BlockKind::whileBlock);
}

void IRVisitor::VisitFor(ASTNode* node)
{
    /// @todo search how to handle then implement + create IRBlock
    m_blockHistory.push(BlockKind::forBlock);
}

void IRVisitor::VisitReturn(ASTNode* node)
{
    if (m_expectedReturnValue)
    {
        m_expectedReturnValue = false;
        m_irbuilder.CreateRet(m_lastSavedValue->GetName());
    }
    else if (m_currFunc.GetReturnType() == IRType::GetVoid())
    {
        m_irbuilder.CreateRetVoid();
    }
    else
    {
        m_expectedReturnValue = true;
    }
}

void IRVisitor::VisitControlStatement(ASTNode* node)
{
    Token statement = node->command[0];

    if (statement.GetValue() == "continue")
    {
        this->m_irbuilder.CreateBr(this->m_currFunc.GetBasicBlocks().back().get());
    }
    else if (statement.GetValue() == "break")
    {
        this->m_irbuilder.CreateBr(this->m_afterLoopBlocks.top());
    }
}

void IRVisitor::VisitBlockStart(ASTNode* node)
{
    /// @todo phi handling + control statement related + move in the varTable

    // whenever entering a loop this command shall be ran
    //this->m_afterLoopBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(this->m_blockHistory.top(), EntranceKind::exit)));
}

void IRVisitor::VisitBlockEnd(ASTNode* node)
{
    /// @todo phi handling + control statement related + move in the varTable

    // whenever exiting a loop we need to make the top of m_afterLoopBlocks the exit block because all the break commands point to it (ownership should be switched to func)
}

void IRVisitor::VisitSizeof(ASTNode* node)
{
    (void)HandleSizeof(node->command);
}

void IRVisitor::VisitFuncDefinition(ASTNode* node)
{
    /// @todo create the func inside the module
}

void IRVisitor::VisitEndOfCommand(ASTNode* node)
{
    /// @todo some of the values need to be reset
}

void IRVisitor::VisitVoidCommand(ASTNode* node)
{
    return;
}

void IRVisitor::VisitPass(ASTNode* node)
{
    return;
}

void IRVisitor::VisitEof(ASTNode* node)
{
    return;
}

/// @todo Add handling of OS
std::shared_ptr<const IRType> IRVisitor::TypeToIRType(Type type)
{
    switch (type)
    {
    case Type::BOOL_TYPE:
        return IRType::GetInt1();
        break;

    case Type::DOUBLE_TYPE:
        return IRType::GetFloat32();
        break;
        
    case Type::INT_TYPE:
        return IRType::GetInt32();
        break;

    case Type::UNSIGNED_INT_TYPE:
        return IRType::GetInt32();
        break;

    case Type::VOID_TYPE:
        return IRType::GetVoid();
        break;
    
    default:
        break;
    }
}

/// @todo Add handling of OS
/// @todo handle ownership of consts
IRBaseValue* IRVisitor::ValueToConstIR(Token token)
{
    std::string value = token.GetValue();

    if (value == "true")
    {
        return this->m_irbuilder.GetBool(true);
    }
    else if (value == "false")
    {
        return this->m_irbuilder.GetBool(false);
    }
    else if (value.find('.') != std::string::npos)
    {
        return this->m_irbuilder.GetFloat(std::stod(value));
    }
    else
    {
        return this->m_irbuilder.GetInt32(std::stoll(value));
    }
}

IRInstruction* IRVisitor::ChooseOperation(Operators usedOperator, IRBaseValue* lsh, IRBaseValue* rsh, bool signage, bool floatness)
{
    switch (usedOperator)
    {
    case Operators::Add:
        if (floatness)
        {
            return this->m_irbuilder.CreateFAdd(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateAdd(lsh, rsh);
        }
        break;
    
    case Operators::Sub:
        if (floatness)
        {
            return this->m_irbuilder.CreateFSub(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateSub(lsh, rsh);
        }
        break;
    
    case Operators::Mul:
        if (floatness)
        {
            return this->m_irbuilder.CreateFMul(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateMul(lsh, rsh);
        }
        break;
    
    case Operators::Div:
        if (signage)
        {
            return this->m_irbuilder.CreateUDiv(lsh, rsh);
        }
        else if (floatness)
        {
            return this->m_irbuilder.CreateFDiv(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateSDiv(lsh, rsh);
        }
        break;
    
    case Operators::Rem:
        if (signage)
        {
            return this->m_irbuilder.CreateURem(lsh, rsh);
        }
        else if (floatness)
        {
            /// @todo Create this func in irbuilder
            return this->m_irbuilder.CreaFRem(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateSRem(lsh, rsh);
        }
        break;
    
    case Operators::And:
        return this->m_irbuilder.CreateAnd(lsh, rsh);
        break;
    
    case Operators::Or:
        return this->m_irbuilder.CreateOr(lsh, rsh);
        break;
    
    case Operators::Xor:
        return this->m_irbuilder.CreateXor(lsh, rsh);
        break;
    
    case Operators::Shl:
        return this->m_irbuilder.CreateShl(lsh, rsh);
        break;
    
    case Operators::LShr:
        return this->m_irbuilder.CreateLshr(lsh, rsh);
        break;
    
    case Operators::AShr:
        return this->m_irbuilder.CreateAShr(lsh, rsh);
        break;
    
    case Operators::Eq:
        if (floatness)
        {
            return this->m_irbuilder.CreateFCmpEQ(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateICmpEQ(lsh, rsh);
        }
        break;
    
    case Operators::Ne:
        if (floatness)
        {
            return this->m_irbuilder.CreateFCmpNE(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateICmpNE(lsh, rsh);
        }
        break;
    
    case Operators::Lt:
        if (signage)
        {
            return this->m_irbuilder.CreateICmpULT(lsh, rsh);
        }
        else if (floatness)
        {
            return this->m_irbuilder.CreateFCmpLT(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateICmpSLT(lsh, rsh); 
        }
        break;
    
    case Operators::Le:
        if (signage)
        {
            return this->m_irbuilder.CreateICmpULE(lsh, rsh);
        }
        else if (floatness)
        {
            return this->m_irbuilder.CreateFCmpLE(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateICmpSLE(lsh, rsh); 
        }
        break;

    case Operators::Gt:
        if (signage)
        {
            return this->m_irbuilder.CreateICmpUGT(lsh, rsh);
        }
        else if (floatness)
        {
            return this->m_irbuilder.CreateFCmpGT(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateICmpSGT(lsh, rsh); 
        }
        break;

    case Operators::Ge:
        if (signage)
        {
            return this->m_irbuilder.CreateICmpUGE(lsh, rsh);
        }
        else if (floatness)
        {
            return this->m_irbuilder.CreateFCmpGE(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateICmpSGE(lsh, rsh); 
        }
        break;
    
    default:
        break;
    }
}

Operators IRVisitor::TokenizerOperatorToIROperator(Token token)
{
    std::string value = token.GetValue();
    if (value == "And")
    {
        return Operators::And;
    }
    else if (value == "or")
    {
        return Operators::Or;
    }
    else if (value == "xor")
    {
        return Operators::Xor;
    }
    else if (value == "==")
    {
        return Operators::Eq;
    }
    else if (value == "//")
    {
        /// @todo think how to do this
    }
    else if (value == "+")
    {
        return Operators::Add;
    }
    else if (value == "-")
    {
        return Operators::Sub;
    }
    else if (value == "*")
    {
        return Operators::Mul;
    }
    else if (value == "/")
    {
        return Operators::Div;
    }
    else if (value == "%")
    {
        return Operators::Rem;
    }
    else if (value == ">")
    {
        return Operators::Gt;
    }
    else if (value == "<")
    {
        return Operators::Lt;
    }
}

bool IRVisitor::CheckSignage(Token token)
{
    return token.GetValue().find('-') != std::string::npos;
}

bool IRVisitor::CheckFloatness(Token token)
{
    return token.GetValue().find('.') != std::string::npos;
}

IRBaseValue* IRVisitor::HandleVariableValue(Command command)
{
    /// @todo Check func correctness
    /// It should get an IdentifierValue using IRBuilder::CreateLoad
    std::string varName = ASTExtractor::ExtractVarName(node);
    const VarMetaData& varMetaData = m_decoratedAst.varTable.FindVar(varName);
    if (!varMetaData) return;
    m_currVarMetaData = varMetaData;

    m_irbuilder.CreateLoad(
        IRBaseValue(
            varMetaData.sName,
            TypeToIRType(
                varMetaData.type
            )
        )
    );
}

IRBaseValue* IRVisitor::HandleFuncCall(Command command)
{
    std::vector<IRBaseValue*> args;
    const FuncMetaData& funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(innerCommand[0]);
    if (!funcMetaData) return;
    const std::vector<VarMetaData>& params = funcMetaData.GetFuncParams();
    // Only if the func has arguements we add them
    if (params.size() != 0 && params[0].type != Type::VOID_TYPE)
    {
        for (auto expression : command.GetParams())
        {
            this->m_funcArgs.top().push_back(HandleExpression(expression));
        }
    }
    IRBaseValue* funcCallValue = this->m_irbuilder.CreateCall(
        this->m_currModule.GetFunction(funcMetaData.GetName()),
        this->m_funcArgs.top()
    );

    this->m_funcArgs.pop();
    return funcCallValue;
}

IRBaseValue* IRVisitor::HandleSizeof(Command command)
{
    /// @todo Create this func.
    /// It should find out the type of the expression in it and return the corresponding size.
    const Command& SizeofArg = node->command.GetParams()[0];
    if (SizeofArg.GetType() == CommandTypes::EXPRESSION)
    {
        
    }
    else // Sizeof with type
    {
        /// @todo here save a constant value
    }
}

IRBaseValue* IRVisitor::HandleExpressionAdding(IRInstruction* oldInstruction, IRInstruction* newInstruction, size_t notCnt, Operators lastOperator, bool signage, bool floatness)
{
    if (!oldInstruction) // Incase it's epxression start
    {
        if (notCnt != 0) // This means the epxression started with nots
        {
            if (notCnt % 2 == 0) // The nots disabled themselves
            {
                return newInstruction;
            }
            else
            {
                return this->m_irbuilder.CreateAnd(
                    this->m_irbuilder.GetBool(false),
                    newInstruction
                );
            }
        }
        else if (lastOperator != Operators::None) // The expression started with a minus '-'
        {
            return this->m_irbuilder.CreateMul(
                ValueToConstIR(
                    Token(
                        "-",
                        TokenTypes::VALUE,
                        0,
                        0
                    )
                ),
                newInstruction
            );
        }
        else // Expression starter
        {
            return newInstruction;
        }
    }
    else if (notCnt != 0)
    {
        if (notCnt % 2 != 0) // The nots didnt disable themselves
        {
            return this->m_irbuilder.CreateAnd(
                this->m_irbuilder.GetBool(false),
                newInstruction
            );
        }

        return ChooseOperation(
            lastOperator,
            oldInstruction,
            newInstruction,
            signage,
            floatness
        );
    }
    else
    {
        return ChooseOperation(
            lastOperator,
            oldInstruction,
            newInstruction,
            signage,
            floatness
        );
    }
}

std::string IRVisitor::GetCorrespondingLabel(BlockKind blockKind, EntranceKind entryKind)
{
    switch (blockKind)
    {
    case BlockKind::elifBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            return "then" + std::to_string(this->m_thenCnt);
            break;

        case EntranceKind::exit:
            return "else" + std::to_string(this->m_elseCnt);
            break;

        default:
            break;
        }
        break;
    
    case BlockKind::elseBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            return "then" + std::to_string(this->m_thenCnt);
            break;

        default:
            break;
        }
        break;

    case BlockKind::forBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            return "for.body" + std::to_string(this->m_bodyCnt);
            break;

        case EntranceKind::exit:
            return "for.exit" + std::to_string(this->m_exitCnt);
            break;

        case EntranceKind::condition:
            return "for.cond" + std::to_string(this->m_condCnt);
            break;
        
        default:
            break;
        }
        break;

    case BlockKind::functionBlock:
        
        break;

    case BlockKind::ifBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            return "then" + std::to_string(this->m_thenCnt);
            break;

        case EntranceKind::exit:
            return "else" + std::to_string(this->m_elseCnt);
            break;

        default:
            break;
        }
        break;

    case BlockKind::whileBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            return "while.body" + std::to_string(this->m_bodyCnt);
            break;

        case EntranceKind::exit:
            return "while.exit" + std::to_string(this->m_exitCnt);
            break;

        case EntranceKind::condition:
            return "while.cond" + std::to_string(this->m_condCnt);
            break;

        default:
            break;
        }
        break;
    
    default:
        break;
    }
}

IRBaseValue* IRVisitor::HandleExpression(Command command)
{
    size_t paramIndex = 0;
    size_t notCnt = 0;
    IRInstruction* lastValue = nullptr;
    Operators lastOperator = Operators::None;
    bool signage = false;
    bool floatness = false;

    for (auto token : command.GetTokens())
    {
        if (token.GetType() == TokenTypes::HAS_PARAM)
        {
            Command innerCommand = command.GetParams()[paramIndex];
            IRBaseValue* rhs = nullptr;
            if (innerCommand.GetType() == CommandTypes::IDENTIFIER_VALUE)
            {
                const VarMetaData& varMetaData = m_decoratedAst.varTable.FindVar(innerCommand[0]);
                if (!varMetaData) return;
                if (varMetaData.type == Type::UNSIGNED_INT_TYPE) signage = true;
                else if (varMetaData.type == Type::DOUBLE_TYPE) floatness = true;
                rhs = HandleVariableValue(innerCommand);
            }
            else if (innerCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
            {
                const FuncMetaData& funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(innerCommand[0]);
                if (!funcMetaData) return;
                if (funcMetaData.GetRType() == Type::UNSIGNED_INT_TYPE) signage = true;
                else if (funcMetaData.GetRType() == Type::DOUBLE_TYPE) floatness = true;
                rhs = HandleFuncCall(innerCommand);
            }
            else if (innerCommand.GetType() == CommandTypes::SIZEOF)
            {
                rhs = HandleSizeof(innerCommand);
            }

            HandleExpressionAdding(lastValue, rhs, notCnt, lastOperator, signage, floatness);
            notCnt = 0; // reset
            paramIndex++;
        }
        else if (token.GetType() == TokenTypes::VALUE)
        {
            if (CheckSignage(token)) signage = true;
            if (CheckFloatness(token)) floatness = true;

            IRBaseValue* value = ValueToConstIR(token);
            IRInstruction* defaultInstruction = this->m_irbuilder.CreateAdd(
                this->m_irbuilder.GetInt32(0),
                value
            );
            HandleExpressionAdding(lastValue, defaultInstruction, notCnt, lastOperator, signage, floatness);
            notCnt = 0; // reset
        }
        else if (token.GetValue() == "not")
        {
            notCnt++;
        }
        else if (token.GetType() == TokenTypes::OPERATOR ||
            token.GetType() == TokenTypes::KEYWORD)
        {
            lastOperator = TokenizerOperatorToIROperator(token);
        }
        /// @todo add parantheses handling
    }

    return lastValue;
}

void IRVisitor::CheckValueNeededSomewhere(IRBaseValue* value, ASTNode* node)
{
    /// @todo add more places where the returned value is needed e.g. assigmnet
    if (this->m_expectedReturnValue)
    {
        this->m_lastSavedValue = value;
        this->VisitReturn(node);
    }
    else if (this->m_expectedAssignment)
    {
        this->m_lastSavedValue = value;
        this->VisitAssignment(node);
    }
}
