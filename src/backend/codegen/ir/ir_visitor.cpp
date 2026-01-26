#include "includes/backend/codegen/ir/ir_visitor.h"

IRVisitor::IRVisitor(
    const EntityLogger& entityLogger,
    ErrorManager& errorManager,
    DecoratedAST& decoratedAst,
    IRModule& module) : 
    m_entityLogger(entityLogger),
    m_errorManager(errorManager),
    m_decoratedAst(decoratedAst),
    m_currModule(module),
    m_irbuilder(m_entityLogger)
{
    this->m_varTableIndex.push(0);
    this->m_currVarTableNode = &this->m_decoratedAst.varTable;
}

void IRVisitor::VisitVarDeclaration(ASTNode* node)
{
    /// Getting the varMetaData
    auto varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(node));
    if (!varMetaData) return;
    m_currVarMetaData = varMetaData; // Saving it incase there will be assigmnet later on

    // The var is saved differently if it is global or local.
    if (varMetaData.value().scope == Scope::GLOBAL)
    {
        m_currModule.AddVariable(
            varMetaData.value().sName,
            TypeToIRType(
                varMetaData.value().type
            )
        );
    }
    else
    {
        // Any allocated var is saved in the entry block
        auto tempSave = this->m_irbuilder.GetInsertBlock();
        this->m_irbuilder.SetInsertPoint(this->m_currFunc->GetEntryBlock());

        m_irbuilder.CreateAlloca(
            TypeToIRType(
                varMetaData.value().type
            ),
            varMetaData.value().sName
        );

        this->m_irbuilder.SetInsertPoint(tempSave); //reset
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
    // Getting the varMetaData and saving it for later use incase of assignment.
    std::optional<VarMetaData> varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(node));
    if (!varMetaData) return;
    m_currVarMetaData = varMetaData;
}

void IRVisitor::VisitIdentifierValue(ASTNode* node)
{
    (void)HandleIdentifierValue(node);
}

void IRVisitor::VisitAssignment(ASTNode* node)
{
    // If this flag is up it means the expression/func_call/idenitifierValue already finished.
    if (this->m_expectedAssignment)
    {
        this->m_irbuilder.CreateStore(
            this->m_lastSavedValue,
            FindPointer(this->m_currVarMetaData.value().sName)
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
    this->m_blockHistory.push(BlockKind::functionBlock);
    // Chking if the func is already defined.
    FuncMetaData* funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(node->command[0].GetValue());
    if (!funcMetaData) return;
    // Creating the func in the module
    IRFunction* createdFunc = this->m_currModule.AddFunction(
        funcMetaData->GetName(),
        TypeToIRType(funcMetaData->GetRType())
    );

    // Adding the args to the func
    for (auto argMetaData : funcMetaData->GetFuncParams())
    {
        createdFunc->AddArgument(argMetaData.sName, TypeToIRType(argMetaData.type));
    }
}

void IRVisitor::VisitFuncCall(ASTNode* node)
{
    (void)HandleFuncCall(node->command);
}

void IRVisitor::VisitIf(ASTNode* node)
{
    m_blockHistory.push(BlockKind::ifBlock);
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::ifBlock, EntranceKind::exit)));
    this->m_conditionMergeBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::elseBlock, EntranceKind::exit)));
    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::ifBlock, EntranceKind::entry));

    IRBaseValue* expressionResult = HandleExpression(node->command.GetParams()[0]);

    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            expressionResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );
}

void IRVisitor::VisitElif(ASTNode* node)
{
    m_blockHistory.push(BlockKind::elifBlock);
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::elifBlock, EntranceKind::exit)));
    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::elifBlock, EntranceKind::entry));

    IRBaseValue* expressionResult = HandleExpression(node->command.GetParams()[0]);

    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            expressionResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );
}

void IRVisitor::VisitElse(ASTNode* node)
{
    m_blockHistory.push(BlockKind::elseBlock);
    this->m_nextExitBlocks.push(this->m_conditionMergeBlocks.top());
}

void IRVisitor::VisitWhile(ASTNode* node)
{
    m_blockHistory.push(BlockKind::whileBlock);
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::whileBlock, EntranceKind::exit)));
    this->m_afterLoopBlocks.push(this->m_nextExitBlocks.top().get());

    auto condBlock = this->m_currFunc->AddBasicBlock(GetCorrespondingLabel(BlockKind::whileBlock, EntranceKind::condition));
    this->m_conditionBlocks.push(condBlock);
    this->m_irbuilder.CreateBr(condBlock);
    this->m_irbuilder.SetInsertPoint(condBlock);

    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::whileBlock, EntranceKind::entry));
    IRBaseValue* expressionResult = HandleExpression(node->command.GetParams()[0]);
    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            expressionResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );
}

void IRVisitor::VisitFor(ASTNode* node)
{
    m_blockHistory.push(BlockKind::forBlock);
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::forBlock, EntranceKind::exit)));
    this->m_afterLoopBlocks.push(this->m_nextExitBlocks.top().get());
    if (node->command.GetParams()[0].GetType() == CommandTypes::VAR_DECLARATION)
    {
        VisitVarDeclaration(new ASTNode(node->command.GetParams()[0]));
    }
    else
    {
        VisitIdentifier(new ASTNode(node->command.GetParams()[0]));
    }
    VisitAssignment(new ASTNode(node->command.GetParams()[1]));
    VisitExpression(new ASTNode(node->command.GetParams()[2]));

    auto condBlock = this->m_currFunc->AddBasicBlock(GetCorrespondingLabel(BlockKind::forBlock, EntranceKind::condition));
    this->m_conditionBlocks.push(condBlock);
    this->m_irbuilder.CreateBr(condBlock);
    this->m_irbuilder.SetInsertPoint(condBlock);

    VisitExpression(new ASTNode(node->command.GetParams()[3]));
    auto forCondResult = GetLastIsntruction();
    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::forBlock, EntranceKind::entry));
    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            forCondResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );

    this->m_forIncrements.push(node);
}

void IRVisitor::VisitReturn(ASTNode* node)
{
    // If we got the Instruction of the return expression
    if (m_expectedReturnValue)
    {
        m_expectedReturnValue = false;
        m_irbuilder.CreateRet(this->m_lastSavedValue);
    }
    else if (m_currFunc->GetReturnType() == IRType::GetVoid())
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
        /// Jumping to the condition block of the loop.
        this->m_irbuilder.CreateBr(this->m_conditionBlocks.top());
    }
    else if (statement.GetValue() == "break")
    {
        // Jumping to the exit block
        this->m_irbuilder.CreateBr(this->m_afterLoopBlocks.top());
    }
}

void IRVisitor::VisitBlockStart(ASTNode* node)
{
    size_t currVarTableIndex = this->m_varTableIndex.top();
    this->m_varTableIndex.pop();
    this->m_varTableIndex.push(currVarTableIndex + 1);
    this->m_currVarTableNode = this->m_currVarTableNode->GetBlock(currVarTableIndex);
    this->m_varTableIndex.push(0);

    switch (this->m_blockHistory.top())
    {
    case BlockKind::functionBlock:
        this->m_irbuilder.SetInsertPoint(this->m_currFunc->AddBasicBlock("entry"));
        break;

    case BlockKind::elseBlock:
        break;

    case BlockKind::forBlock:
    case BlockKind::whileBlock:
    case BlockKind::elifBlock:
    case BlockKind::ifBlock:
        this->m_currFunc->AddBasicBlock(std::move(this->m_nextControlEntryBlock));
        this->m_irbuilder.SetInsertPoint(this->m_nextControlEntryBlock.get());
        this->m_nextControlEntryBlock = nullptr;
        break;
    
    default:
        break;
    }
}

void IRVisitor::VisitBlockEnd(ASTNode* node)
{
    Command forCommand;
    ASTNode* forNode;
    BasicBlock* tempExitBlockHolder;
    this->m_varTableIndex.pop();
    this->m_currVarTableNode = this->m_currVarTableNode->GetParent();

    switch (this->m_blockHistory.top())
    {
    case BlockKind::functionBlock:
        this->m_currFunc = nullptr;
        /// @todo check if resets needed
        m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(nullptr);
        break;

    case BlockKind::forBlock:
        forNode = this->m_forIncrements.top();
        this->m_forIncrements.pop();
        forCommand = forNode->command;
        VisitIdentifier(new ASTNode(forCommand.GetParams()[4]));
        VisitAssignment(new ASTNode(forCommand.GetParams()[5]));
        VisitExpression(new ASTNode(forCommand.GetParams()[6]));
    case BlockKind::whileBlock:
        this->m_irbuilder.CreateBr(this->m_conditionBlocks.top());
        this->m_conditionBlocks.pop();
        this->m_afterLoopBlocks.pop();
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        break;

    case BlockKind::ifBlock:
    case BlockKind::elifBlock:
        this->m_irbuilder.CreateBr(this->m_conditionMergeBlocks.top().get());
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        if (node->nodeNextSibling->command.GetType() != CommandTypes::IF ||
            node->nodeNextSibling->command.GetType() != CommandTypes::ELIF ||
            node->nodeNextSibling->command.GetType() != CommandTypes::ELSE)
        {
            tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_conditionMergeBlocks.top()));
            this->m_conditionMergeBlocks.pop();
            this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        }
        break;

    case BlockKind::elseBlock:
        this->m_irbuilder.CreateBr(this->m_nextExitBlocks.top().get());
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_conditionMergeBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        break;
    
    default:
        break;
    }
}

void IRVisitor::VisitSizeof(ASTNode* node)
{
    (void)HandleSizeof(node->command);
}

void IRVisitor::VisitFuncDefinition(ASTNode* node)
{
    // Declaring the func incase it is def+dec (double definition cant happen)
    VisitFuncDeclaration(node);
    // Changing the m_currFunc to the new one. 
    IRFunction* foundFunc = this->m_currModule.GetFunction(node->command[0].GetValue());
    if (!foundFunc) return;
    this->m_currFunc = foundFunc;
    this->m_blockHistory.push(BlockKind::functionBlock);
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

std::shared_ptr<const IRType> TypeToIRType(const std::string& type)
{
    if (type == "bool")
    {
        return IRType::GetInt1();
    }

    if (type == "double")
    {
        return IRType::GetFloat32();
    }

    if (type == "int" || type == "unsigned int")
    {
        return IRType::GetInt32();
    }

    if (type == "void")
    {
        return IRType::GetVoid();
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

IRInstruction* IRVisitor::ChooseOperation(
    Operators usedOperator,
    IRBaseValue* lsh,
    IRBaseValue* rsh,
    bool signage,
    bool floatness
)
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
            return this->m_irbuilder.CreateFRem(lsh, rsh);
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
        return this->m_irbuilder.CreateLShr(lsh, rsh);
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
    return token.GetValue().find('-') == std::string::npos;
}

bool IRVisitor::CheckFloatness(Token token)
{
    return token.GetValue().find('.') != std::string::npos;
}

IRBaseValue* IRVisitor::HandleIdentifierValue(ASTNode* node)
{
    const std::optional<VarMetaData>& varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(node));
    if (!varMetaData) return nullptr;
    m_currVarMetaData = varMetaData;

    return m_irbuilder.CreateLoad(
        FindPointer(this->m_currVarMetaData.value().sName)
    );
}

IRBaseValue* IRVisitor::HandleFuncCall(Command command)
{
    // Getting the func parameters
    FuncMetaData* funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(command[0].GetValue());
    if (!funcMetaData) return nullptr;
    const std::vector<VarMetaData>& params = funcMetaData->GetFuncParams();

    // Setting up the temporary holder
    this->m_funcArgs.push(std::vector<IRBaseValue*>());

    // Only if the func has arguements we add them
    if (params.size() != 0 && params[0].type != Type::VOID_TYPE)
    {
        for (auto expression : command.GetParams())
        {
            this->m_funcArgs.top().push_back(HandleExpression(expression));
        }
    }

    // Creating the Call Instruction
    IRBaseValue* funcCallValue = this->m_irbuilder.CreateCall(
        this->m_currModule.GetFunction(funcMetaData.GetName()),
        this->m_funcArgs.top()
    );

    this->m_funcArgs.pop(); // Removing the temporary holder
    return funcCallValue;
}

/// @todo add os support
IRBaseValue* IRVisitor::HandleSizeof(Command command)
{
    std::shared_ptr<const IRType> sizeofType = nullptr;

    const Command& SizeofArg = command.GetParams()[0];
    if (SizeofArg.GetType() == CommandTypes::EXPRESSION ||
        SizeofArg.GetType() == CommandTypes::FINISHED_FUNC_CALL ||
        SizeofArg.GetType() == CommandTypes::IDENTIFIER_VALUE
    )
    {
        IRBaseValue* value = HandleExpression(command.GetParams()[0]);
        sizeofType = value->GetType();
    }
    else // Sizeof with type
    {
        sizeofType = TypeToIRType(command.GetParams()[0][0].GetValue());
    }

    if (sizeofType == IRType::GetInt32())
    {
        return this->m_irbuilder.GetInt32(4);
    }
    else if (sizeofType == IRType::GetFloat32())
    {
        return this->m_irbuilder.GetInt32(4);
    }
    else if (sizeofType == IRType::GetInt1())
    {
        return this->m_irbuilder.GetInt32(1);
    }
    else if (sizeofType == IRType::GetVoid())
    {
        return this->m_irbuilder.GetInt32(0);
    }
    else
    {
        /// @todo GetFunc handling
    }
}

IRBaseValue* IRVisitor::HandleExpressionToInstruction(
    IRBaseValue* oldInstruction,
    IRBaseValue* newInstruction,
    size_t notCnt,
    Operators lastOperator,
    bool signage,
    bool floatness)
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
            this->m_thenCnt++;
            break;

        case EntranceKind::exit:
            return "else" + std::to_string(this->m_elseCnt);
            this->m_elseCnt++;
            break;

        default:
            break;
        }
        break;
    
    case BlockKind::elseBlock:
        switch (entryKind)
        {
        case EntranceKind::exit:
            return "merge" + std::to_string(this->m_mergeCnt);
            this->m_mergeCnt++;
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
            this->m_bodyCnt++;
            break;

        case EntranceKind::exit:
            return "for.exit" + std::to_string(this->m_exitCnt);
            this->m_exitCnt++;
            break;

        case EntranceKind::condition:
            return "for.cond" + std::to_string(this->m_condCnt);
            this->m_condCnt++;
            break;
        
        default:
            break;
        }
        break;

    case BlockKind::functionBlock:
        return "entry";
        break;

    case BlockKind::ifBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            return "then" + std::to_string(this->m_thenCnt);
            this->m_thenCnt++;
            break;

        case EntranceKind::exit:
            return "else" + std::to_string(this->m_elseCnt);
            this->m_elseCnt++;
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
            this->m_bodyCnt++;
            break;

        case EntranceKind::exit:
            return "while.exit" + std::to_string(this->m_exitCnt);
            this->m_exitCnt++;
            break;

        case EntranceKind::condition:
            return "while.cond" + std::to_string(this->m_condCnt);
            this->m_condCnt++;
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

    if (command.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        return HandleIdentifierValue(new ASTNode(command));
    }
    else if (command.GetType() == CommandTypes::IDENTIFIER_VALUE)
    {
        return HandleFuncCall(command);
    }

    for (auto token : command.GetTokens())
    {
        // IDENTIFIER_VALUE FINISHED_FUNC_CALL or SIZEOF
        if (token.GetType() == TokenTypes::HAS_PARAM)
        {
            Command innerCommand = command.GetParams()[paramIndex];
            IRBaseValue* rhs = nullptr;

            if (innerCommand.GetType() == CommandTypes::IDENTIFIER_VALUE)
            {
                const std::optional<VarMetaData> varMetaData = this->m_currVarTableNode->FindVar(innerCommand[0].GetValue());
                if (!varMetaData) return nullptr;

                // Checking if unsigned or float
                if (varMetaData.value().type == Type::UNSIGNED_INT_TYPE) signage = true;
                else if (varMetaData.value().type == Type::DOUBLE_TYPE) floatness = true;

                ASTNode tempNode = ASTNode(innerCommand); // Temp holder
                rhs = HandleIdentifierValue(new ASTNode(innerCommand));
            }
            else if (innerCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
            {
                FuncMetaData* funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(innerCommand[0].GetValue());
                if (!funcMetaData) return nullptr;

                // Checking if unsigned or float
                if (funcMetaData.GetRType() == Type::UNSIGNED_INT_TYPE) signage = true;
                else if (funcMetaData.GetRType() == Type::DOUBLE_TYPE) floatness = true;

                rhs = HandleFuncCall(innerCommand);
            }
            else if (innerCommand.GetType() == CommandTypes::SIZEOF)
            {
                rhs = HandleSizeof(innerCommand);
            }

            HandleExpressionToInstruction(
                lastValue,
                rhs,
                notCnt,
                lastOperator,
                signage,
                floatness
            );
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
            HandleExpressionToInstruction(
                lastValue,
                defaultInstruction,
                notCnt,
                lastOperator,
                signage,
                floatness
            );
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

inline IRInstruction* IRVisitor::GetLastIsntruction()
{
    return this->m_currFunc->GetBasicBlocks().back().get()->GetInstructions().back().get();
}

IRBaseValue* IRVisitor::FindPointer(const std::string& name)
{
    if (this->m_currFunc->FindAllocatedVar(name))
    {
        return this->m_currFunc->FindAllocatedVar(name);
    }
    else if (this->m_currModule.GetVariable(name))
    {
        return this->m_currModule.GetVariable(name);
    }
    else if (this->m_currFunc->GetArgument(name))
    {
        return this->m_currFunc->GetArgument(name);
    }
    else
    {
        /// @todo error here
    }
}
