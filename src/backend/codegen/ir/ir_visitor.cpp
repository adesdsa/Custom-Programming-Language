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
    m_irbuilder(m_entityLogger),
    m_thenCnt(0),
    m_elseCnt(0),
    m_condCnt(0),
    m_bodyCnt(0),
    m_exitCnt(0),
    m_mergeCnt(0),
    m_nextControlEntryBlock(nullptr),
    m_lastSavedValue(nullptr),
    m_currVarMetaData(std::nullopt),
    m_expectedReturnValue(false),
    m_expectedAssignment(false),
    m_returnedAlready(false)
{
    this->m_varTableIndex.push(0);
    this->m_currVarTableNode = &this->m_decoratedAst.varTable;
}

std::vector<std::unique_ptr<IRBaseValue>> IRVisitor::DumpConstants() noexcept
{
    auto tempConstHolder = this->m_irbuilder.DumpConstants();
    this->m_irbuilder.ClearConstants();
    return std::move(tempConstHolder);
}

void IRVisitor::VisitVarDeclaration(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    /// Getting the varMetaData
    auto varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(node));
    if (!varMetaData) 
    {
        StoreInternalError("IRVisitor::VisitVarDeclaration unknown var");
        return;
    }
    this->m_currVarMetaData = varMetaData; // Saving it incase there will be assigmnet later on

    // The var is saved differently if it is global or local.
    if (varMetaData.value().scope == Scope::GLOBAL)
    {
        this->m_currModule.AddVariable(
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

        this->m_irbuilder.CreateAlloca(
            TypeToIRType(
                varMetaData.value().type
            ),
            varMetaData.value().sName
        );

        this->m_irbuilder.SetInsertPoint(tempSave); //reset
    }
    this->m_entityLogger.Info("Added VarDeclaration IR handling");
}

void IRVisitor::VisitExpression(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    IRBaseValue* expressionValue = HandleExpression(node->command);
    CheckValueNeededSomewhere(expressionValue, node);
    this->m_entityLogger.Info("Added Expression IR handling");
}

void IRVisitor::VisitIdentifier(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    // Getting the varMetaData and saving it for later use incase of assignment.
    std::optional<VarMetaData> varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(node));
    if (!varMetaData)
    {
        StoreInternalError("IRVisitor::VisitIdentifier unknown var");
        return;
    }
    this->m_currVarMetaData = varMetaData;
    this->m_entityLogger.Info("Added Identifier IR handling");
}

void IRVisitor::VisitIdentifierValue(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    auto loadInstruction = HandleIdentifierValue(node);
    CheckValueNeededSomewhere(loadInstruction, node);
    this->m_entityLogger.Info("Added IdentifierValue IR handling");
}

void IRVisitor::VisitAssignment(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
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
    this->m_entityLogger.Info("Added Assignment IR handling");
}

void IRVisitor::VisitFuncDeclaration(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    this->m_blockHistory.push(BlockKind::functionBlock);
    // Chking if the func is already defined.
    FuncMetaData* funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(node->command[FUNC_IDENTIFIER_INDEX].GetValue());
    if (!funcMetaData)
    {
        StoreInternalError("IRVisitor::VisitFuncDeclaration unknown func");
        return;
    }

    // Getting the arguements' types.
    std::vector<std::shared_ptr<const IRType>> funcParams;
    for (auto argMetaData : funcMetaData->GetFuncParams())
    {
        funcParams.push_back(TypeToIRType(argMetaData.type));
    }

    IRFunction* createdFunc = this->m_currModule.AddFunction(
        funcMetaData->GetName(),
        IRType::GetFunction(
            TypeToIRType(funcMetaData->GetRType()),
            funcParams
        )
    );

    // Adding the args to the func
    for (auto argMetaData : funcMetaData->GetFuncParams())
    {
        createdFunc->AddArgument(
            argMetaData.sName,
            TypeToIRType(argMetaData.type)
        );
    }
    this->m_entityLogger.Info("Added FuncDeclaration IR handling");
}

void IRVisitor::VisitFuncCall(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    auto funcCallInstruction = HandleFuncCall(node->command);
    CheckValueNeededSomewhere(funcCallInstruction, node);
    this->m_entityLogger.Info("Added FuncCall IR handling");
}

void IRVisitor::VisitIf(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    this->m_blockHistory.push(BlockKind::ifBlock);
    /// @see Control Flow Fields docs for more info
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::ifBlock, EntranceKind::exit)));
    this->m_conditionMergeBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::elseBlock, EntranceKind::exit)));
    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::ifBlock, EntranceKind::entry));

    IRBaseValue* expressionResult = HandleExpression(node->command.GetParams()[CONDITION_INDEX]);

    // If expression is true go to the 'then' block else go to the 'else' block
    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            expressionResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );
    this->m_entityLogger.Info("Added If IR handling");
}

void IRVisitor::VisitElif(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    this->m_blockHistory.push(BlockKind::elifBlock);
    /// @see Control Flow Fields docs for more info
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::elifBlock, EntranceKind::exit)));
    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::elifBlock, EntranceKind::entry));

    IRBaseValue* expressionResult = HandleExpression(node->command.GetParams()[CONDITION_INDEX]);

    // If expression is true go to the 'then' block else go to the 'else' block
    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            expressionResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );
    this->m_entityLogger.Info("Added Elif IR handling");
}

void IRVisitor::VisitElse(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    this->m_blockHistory.push(BlockKind::elseBlock);
    this->m_nextExitBlocks.push(std::move(this->m_conditionMergeBlocks.top()));
    this->m_entityLogger.Info("Added Else IR handling");
}

void IRVisitor::VisitWhile(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    this->m_blockHistory.push(BlockKind::whileBlock);
    /// @see Control Flow Fields docs for more info
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::whileBlock, EntranceKind::exit)));
    this->m_afterLoopBlocks.push(this->m_nextExitBlocks.top().get());

    // First we create the codition block and set it up in all the related places.
    auto condBlock = this->m_currFunc->AddBasicBlock(GetCorrespondingLabel(BlockKind::whileBlock, EntranceKind::condition));
    this->m_conditionBlocks.push(condBlock);
    this->m_irbuilder.CreateBr(condBlock);
    this->m_irbuilder.SetInsertPoint(condBlock);

    /// @see Control Flow Fields docs for more info
    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::whileBlock, EntranceKind::entry));

    // Creating the condition expression and jumping to the while.body if true to while.exit if false.
    IRBaseValue* expressionResult = HandleExpression(node->command.GetParams()[CONDITION_INDEX]);
    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            expressionResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );
    this->m_entityLogger.Info("Added While IR handling");
}

void IRVisitor::VisitFor(ASTNode* node)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // For Command is build like this:                                                                                  |
    // for ( Identifier/VarDeclaration + Assigment + Expression + Expression + Identifier + Assignment + Expression )   |
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (this->m_returnedAlready) return; // Incase already returned
    this->m_blockHistory.push(BlockKind::forBlock);
    /// @see Control Flow Fields docs for more info
    this->m_nextExitBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::forBlock, EntranceKind::exit)));
    this->m_afterLoopBlocks.push(this->m_nextExitBlocks.top().get());

    // Building the first Command in the for loop which is Identifier/VarDeclaration + Assigment + Expression
    // That runs before the loop.
    if (node->command.GetParams()[FOR_PRE_COND_VAR_INDEX].GetType() == CommandTypes::VAR_DECLARATION)
    {
        ASTNode tempNode(node->command.GetParams()[FOR_PRE_COND_VAR_INDEX]);
        VisitVarDeclaration(&tempNode);
    }
    else
    {
        ASTNode tempNode(node->command.GetParams()[FOR_PRE_COND_VAR_INDEX]);
        VisitIdentifier(&tempNode);
    }
    ASTNode tempNode(node->command.GetParams()[FOR_PRE_COND_ASSIGNMENT_INDEX]);
    VisitAssignment(&tempNode);
    tempNode = ASTNode(node->command.GetParams()[FOR_PRE_COND_EXPRESSION_INDEX]);
    VisitExpression(&tempNode);

    // Then we create the codition block and set it up in all the related places.
    auto condBlock = this->m_currFunc->AddBasicBlock(GetCorrespondingLabel(BlockKind::forBlock, EntranceKind::condition));
    this->m_conditionBlocks.push(condBlock);
    this->m_irbuilder.CreateBr(condBlock);
    this->m_irbuilder.SetInsertPoint(condBlock);

    /// @see Control Flow Fields docs for more info
    this->m_nextControlEntryBlock = std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::forBlock, EntranceKind::entry));

    tempNode = ASTNode(node->command.GetParams()[FOR_COND_INDEX]);
    // Creating the condition expression inside the for.cond block
    VisitExpression(&tempNode);
    auto forCondResult = GetLastIsntruction();
    this->m_irbuilder.CreateCondBr(
        this->m_irbuilder.CreateAnd(
            forCondResult,
            this->m_irbuilder.GetBool(true)
        ),
        this->m_nextControlEntryBlock.get(),
        this->m_nextExitBlocks.top().get()
    );
    // The for loop increments is handled elsewhere (the i++ part)
    this->m_forIncrements.push(node);
    this->m_entityLogger.Info("Added For IR handling");
}

void IRVisitor::VisitReturn(ASTNode* node)
{
    this->m_returnedAlready = true;
    // If we got the Instruction of the return expression
    if (this->m_expectedReturnValue)
    {
        this->m_expectedReturnValue = false;
        this->m_irbuilder.CreateRet(this->m_lastSavedValue);
    }
    else if (m_currFunc->GetReturnType() == IRType::GetVoid())
    {
        this->m_irbuilder.CreateRetVoid();
    }
    else
    {
        this->m_expectedReturnValue = true;
    }
    this->m_entityLogger.Info("Added Return IR handling");
}

void IRVisitor::VisitControlStatement(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    Token statement = node->command[STATEMENT_INDEX];

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
    this->m_entityLogger.Info("Added ControlStatement IR handling");
}

void IRVisitor::VisitBlockStart(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned

    // Moving the VarTable to the correct node.
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
        // Setting the exit block as the current one.
        this->m_currFunc->AddBasicBlock(std::move(this->m_nextControlEntryBlock));
        this->m_irbuilder.SetInsertPoint(this->m_nextControlEntryBlock.get());
        this->m_nextControlEntryBlock = nullptr;
        break;
    
    default:
        break;
    }
    this->m_entityLogger.Info("Added BlockStart IR handling");
}

void IRVisitor::VisitBlockEnd(ASTNode* node)
{
    if (this->m_returnedAlready)
    {
        this->m_returnedAlready = false;
    }

    Command forCommand;
    ASTNode* forNode;
    BasicBlock* tempExitBlockHolder;
    ASTNode tempNode(node->command);

    // Moving the VarTable to the correct node.
    this->m_varTableIndex.pop();
    this->m_currVarTableNode = this->m_currVarTableNode->GetParent();

    switch (this->m_blockHistory.top())
    {
    case BlockKind::functionBlock:
        /// ====== Resets ======
        this->m_currFunc = nullptr;
        this->m_irbuilder.SetInsertPoint(nullptr);

        this->m_thenCnt = 0;
        this->m_elseCnt = 0;
        this->m_condCnt = 0;
        this->m_bodyCnt = 0;
        this->m_exitCnt = 0;
        this->m_mergeCnt = 0;

        while (!this->m_blockHistory.empty())
        {
            this->m_blockHistory.pop();
        }
        while (!this->m_nextExitBlocks.empty())
        {
            this->m_nextExitBlocks.pop();
        }
        while (!this->m_afterLoopBlocks.empty())
        {
            this->m_afterLoopBlocks.pop();
        }
        while (!this->m_forIncrements.empty())
        {
            this->m_forIncrements.pop();
        }
        while (!this->m_conditionBlocks.empty())
        {
            this->m_conditionBlocks.pop();
        }
        while (!this->m_conditionMergeBlocks.empty())
        {
            this->m_conditionMergeBlocks.pop();
        }
        while (!this->m_varTableIndex.empty())
        {
            this->m_varTableIndex.pop();
        }
        this->m_nextControlEntryBlock = nullptr;
        VisitEndOfCommand(node); // More resets here
        break;

    case BlockKind::forBlock:
        // First we create the increment part of the for loop.
        forNode = this->m_forIncrements.top();
        this->m_forIncrements.pop();
        forCommand = forNode->command;
        tempNode = (forCommand.GetParams()[FOR_PRE_AFTER_COND_VAR_INDEX]);
        VisitIdentifier(&tempNode);
        tempNode = ASTNode(forCommand.GetParams()[FOR_PRE_AFTER_COND_ASSIGNMENT_INDEX]);
        VisitAssignment(&tempNode);
        tempNode = ASTNode(forCommand.GetParams()[FOR_PRE_AFTER_COND_EXPRESSION_INDEX]);
        VisitExpression(&tempNode);

    case BlockKind::whileBlock:
        // Jumping back to the loop.cond block.
        this->m_irbuilder.CreateBr(this->m_conditionBlocks.top());
        this->m_conditionBlocks.pop();
        this->m_afterLoopBlocks.pop(); // reset; no longer needed for break

        // Creating the loop exit block
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        break;

    case BlockKind::ifBlock:
    case BlockKind::elifBlock:
        // Creating a jump to the merge block.
        this->m_irbuilder.CreateBr(this->m_conditionMergeBlocks.top().get());
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);

        // If this is last if or elif merge block is supposed to be here.
        if (node->nodeNextSibling->command.GetType() != CommandTypes::IF &&
            node->nodeNextSibling->command.GetType() != CommandTypes::ELIF &&
            node->nodeNextSibling->command.GetType() != CommandTypes::ELSE)
        {
            tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_conditionMergeBlocks.top()));
            this->m_conditionMergeBlocks.pop();
            this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        }
        break;

    case BlockKind::elseBlock:
        // Creating the jump to the exit block
        this->m_irbuilder.CreateBr(this->m_nextExitBlocks.top().get());
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_conditionMergeBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        break;
    
    default:
        break;
    }
    this->m_blockHistory.pop(); // Reset
    this->m_entityLogger.Info("Added BlockEnd IR handling");
}

void IRVisitor::VisitSizeof(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    auto tempSizeofInstruction = HandleSizeof(node->command);
    CheckValueNeededSomewhere(tempSizeofInstruction, node);
    this->m_entityLogger.Info("Added Sizeof IR handling");
}

void IRVisitor::VisitFuncDefinition(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    // Declaring the func incase it is def+dec (double definition cant happen)
    VisitFuncDeclaration(node);
    // Changing the m_currFunc to the new one. 
    IRFunction* foundFunc = this->m_currModule.GetFunction(node->command[FUNC_IDENTIFIER_INDEX].GetValue());
    if (!foundFunc)
    {
        StoreInternalError("IRVisitor::VisitFuncDefinition unknown func");
        return;
    }
    this->m_currFunc = foundFunc;
    this->m_blockHistory.push(BlockKind::functionBlock);
    this->m_entityLogger.Info("Added FuncDefinition IR handling");
}

void IRVisitor::VisitEndOfCommand(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    // Only resetting.
    this->m_lastSavedValue = nullptr;
    this->m_currVarMetaData = std::nullopt;
    this->m_expectedReturnValue = false;
    this->m_expectedAssignment = false;
    this->m_entityLogger.Info("Added EndOfCommand IR handling");
}

void IRVisitor::VisitVoidCommand(ASTNode* node)
{
    this->m_entityLogger.Info("Added VoidCommand IR handling");
    return;
}

void IRVisitor::VisitPass(ASTNode* node)
{
    this->m_entityLogger.Info("Added Pass IR handling");
    return;
}

void IRVisitor::VisitEof(ASTNode* node)
{
    this->m_entityLogger.Info("Added Eof IR handling");
    return;
}

std::shared_ptr<const IRType> IRVisitor::TypeToIRType(Type type)
{
    switch (type)
    {
    case Type::BOOL_TYPE:
        return IRType::GetInt1();
        break;

    case Type::DOUBLE_TYPE:
        if (TargetArchitecture::Is32Bit()) return IRType::GetFloat32();
        else return IRType::GetFloat64();
        break;
        
    case Type::INT_TYPE:
        if (TargetArchitecture::Is32Bit()) return IRType::GetInt32();
        else return IRType::GetInt64();
        break;

    case Type::UNSIGNED_INT_TYPE:
        if (TargetArchitecture::Is32Bit()) return IRType::GetInt32();
        else return IRType::GetInt64();
        break;

    case Type::VOID_TYPE:
        return IRType::GetVoid();
        break;
    
    default:
        StoreInternalError("IRVisitor::TypeToIRType got an illegal type");
        return IRType::GetVoid();
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
        if (TargetArchitecture::Is32Bit()) return IRType::GetFloat32();
        else return IRType::GetFloat64();
    }

    if (type == "int" || type == "unsigned int")
    {
        if (TargetArchitecture::Is32Bit()) return IRType::GetInt32();
        else return IRType::GetInt64();
    }

    if (type == "void")
    {
        return IRType::GetVoid();
    }
}

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
        if (TargetArchitecture::Is32Bit()) return this->m_irbuilder.GetFloat(std::stod(value));
        else return this->m_irbuilder.GetDouble(std::stod(value));
    }
    else
    {
        if (TargetArchitecture::Is32Bit()) return this->m_irbuilder.GetInt32(std::stoll(value));
        else return this->m_irbuilder.GetInt64(std::stoll(value));
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
    else return Operators::None;
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
    // Finding the variable's name.
    const std::optional<VarMetaData>& varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(node));
    if (!varMetaData)
    {
        StoreInternalError("IRVisitor::HandleIdentifierValue unknown var");
        return nullptr;
    }

    return this->m_irbuilder.CreateLoad(
        FindPointer(varMetaData.value().sName)
    );
}

IRBaseValue* IRVisitor::HandleFuncCall(Command command)
{
    // Getting the expected func parameters
    FuncMetaData* funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(command[FUNC_IDENTIFIER_INDEX].GetValue());
    if (!funcMetaData)
    {
        StoreInternalError("IRVisitor::HandleFuncCall unknown func");
        return nullptr;
    }
    const std::vector<VarMetaData>& params = funcMetaData->GetFuncParams();

    // Setting up the temporary holder
    std::vector<IRBaseValue*> tempInstructionHolder;

    // Only if the func has arguements we add them
    if (params.size() != 0 && params[0].type != Type::VOID_TYPE)
    {
        // Going through all the given func call arguements turning them into Isntructions and saving them.
        for (auto expression : command.GetParams())
        {
            tempInstructionHolder.push_back(HandleExpression(expression));
        }
    }

    // Creating the Call Instruction
    IRBaseValue* funcCallValue = this->m_irbuilder.CreateCall(
        this->m_currModule.GetFunction(funcMetaData->GetName()),
        tempInstructionHolder
    );

    return funcCallValue;
}

IRBaseValue* IRVisitor::HandleSizeof(Command command)
{
    std::shared_ptr<const IRType> sizeofType = nullptr;

    // Incase of expression in sizeof
    const Command& SizeofArg = command.GetParams()[STATEMENT_INDEX];
    if (SizeofArg.GetType() == CommandTypes::EXPRESSION ||
        SizeofArg.GetType() == CommandTypes::FINISHED_FUNC_CALL ||
        SizeofArg.GetType() == CommandTypes::IDENTIFIER_VALUE
    )
    {
        IRBaseValue* value = HandleExpression(command.GetParams()[STATEMENT_INDEX]);
        sizeofType = value->GetType();
    }
    else // Sizeof with type
    {
        sizeofType = TypeToIRType(command.GetParams()[STATEMENT_INDEX][TOKEN_TYPE_INDEX].GetValue());
    }

    // Giving the correct sizeof Value (Those are constants)
    if (sizeofType == IRType::GetInt32())
    {
        return this->m_irbuilder.GetInt32(4);
    }
    else if (sizeofType == IRType::GetFloat32())
    {
        return this->m_irbuilder.GetInt32(4);
    }
    if (sizeofType == IRType::GetInt64())
    {
        return this->m_irbuilder.GetInt32(8);
    }
    else if (sizeofType == IRType::GetFloat64())
    {
        return this->m_irbuilder.GetInt32(8);
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
        StoreInternalError("IRVisitor::HandleSizeof Couldn't find valid type");
        return(nullptr);
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
            this->m_thenCnt++;
            return "then" + std::to_string(this->m_thenCnt - 1);
            break;

        case EntranceKind::exit:
            this->m_elseCnt++;
            return "else" + std::to_string(this->m_elseCnt - 1);
            break;

        default:
            break;
        }
        break;
    
    case BlockKind::elseBlock:
        switch (entryKind)
        {
        case EntranceKind::exit:
            this->m_mergeCnt++;
            return "merge" + std::to_string(this->m_mergeCnt - 1);
            break;

        default:
            break;
        }
        break;

    case BlockKind::forBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            this->m_bodyCnt++;
            return "for.body" + std::to_string(this->m_bodyCnt - 1);
            break;

        case EntranceKind::exit:
            this->m_exitCnt++;
            return "for.exit" + std::to_string(this->m_exitCnt - 1);
            break;

        case EntranceKind::condition:
            this->m_condCnt++;
            return "for.cond" + std::to_string(this->m_condCnt - 1);
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
            this->m_thenCnt++;
            return "then" + std::to_string(this->m_thenCnt - 1);
            break;

        case EntranceKind::exit:
            this->m_elseCnt++;
            return "else" + std::to_string(this->m_elseCnt - 1);
            break;

        default:
            break;
        }
        break;

    case BlockKind::whileBlock:
        switch (entryKind)
        {
        case EntranceKind::entry:
            this->m_bodyCnt++;
            return "while.body" + std::to_string(this->m_bodyCnt - 1);
            break;

        case EntranceKind::exit:
            this->m_exitCnt++;
            return "while.exit" + std::to_string(this->m_exitCnt - 1);
            break;

        case EntranceKind::condition:
            this->m_condCnt++;
            return "while.cond" + std::to_string(this->m_condCnt - 1);
            break;

        default:
            break;
        }
        break;
    
    default:
        break;
    }
}

static bool IsOneOf(Operators op, std::initializer_list<Operators> set)
{
    for (Operators currCheck : set) if (op == currCheck) return true;
    return false;
}

IRBaseValue* IRVisitor::ApplyNotIfNeeded(IRBaseValue* value, size_t notCnt)
{
    if (!value)
    {
        StoreInternalError("IRVisitor::ApplyNotIfNeeded value somehow is null");
        return nullptr;
    }
    if (notCnt % 2 == 0) return value;
    return this->m_irbuilder.CreateAnd(this->m_irbuilder.GetBool(false), value);
}

void IRVisitor::ReduceExprByOperators(std::vector<ExprItem>& expr, std::initializer_list<Operators> ops)
{
    if (expr.size() < 2) return;

    for (size_t i = 0; i + 1 < expr.size(); /*advance inside*/)
    {
        Operators currOp = expr[i].op;
        if (!IsOneOf(currOp, ops))
        {
            ++i;
            continue;
        }

        // Apply 'not' individually to lhs/rhs before calling ChooseOperation
        IRBaseValue* lhs = ApplyNotIfNeeded(expr[i].value, expr[i].notCnt);
        IRBaseValue* rhs = ApplyNotIfNeeded(expr[i + 1].value, expr[i + 1].notCnt);

        bool combinedSignage = expr[i].signage || expr[i + 1].signage;
        bool combinedFloatness = expr[i].floatness || expr[i + 1].floatness;

        IRBaseValue* result = ChooseOperation(
            currOp,
            lhs,
            rhs,
            combinedSignage,
            combinedFloatness
        );

        // replace item i with result and inherit operator from i+1
        expr[i].value = result;
        expr[i].signage = combinedSignage;
        expr[i].floatness = combinedFloatness;
        expr[i].notCnt = 0;
        expr[i].op = expr[i + 1].op;

        // erase the consumed item
        expr.erase(expr.begin() + (i + 1));
        // do not advance i — the new expr[i].op might also be in ops
    }
}

/// Full precedence reduction order for Operators enum:
/// 1) Mul/Div/Rem
/// 2) Add/Sub
/// 3) Lt/Gt/Ge/Le
/// 4) Eq/Ne
/// 5) And
/// 6) Xor
/// 7) Or
void IRVisitor::ReduceExprByPrecedence(std::vector<ExprItem>& expr)
{
    ReduceExprByOperators(expr, { Operators::Mul, Operators::Div, Operators::Rem });
    ReduceExprByOperators(expr, { Operators::Add, Operators::Sub });
    ReduceExprByOperators(expr, { Operators::Lt, Operators::Gt , Operators::Ge, Operators::Le});
    ReduceExprByOperators(expr, { Operators::Eq, Operators::Ne });
    ReduceExprByOperators(expr, { Operators::And });
    ReduceExprByOperators(expr, { Operators::Xor });
    ReduceExprByOperators(expr, { Operators::Or });
}

/// Parse a token vector into a flat vector<ExprItem>, handling parentheses recursively.
/// Advances index 'i' as it consumes tokens. paramIndex advances on HAS_PARAM tokens.
std::vector<ExprItem> IRVisitor::CollectExprItems(
    const Command& command,
    const std::vector<Token>& tokens,
    size_t& tokenIndex,
    size_t& paramIndex)
{
    std::vector<ExprItem> expr;
    size_t pendingNotCnt = 0;

    while (tokenIndex < tokens.size())
    {
        const Token& token = tokens[tokenIndex++];

        // Open parenthesis -> recursively parse until matching ')'
        if (token.GetValue() == "(")
        {
            std::vector<ExprItem> inner = CollectExprItems(command, tokens, tokenIndex, paramIndex);
            if (inner.empty())
            {
                // empty parantheses
                return {};
            }
            // reduce the inner expression so it becomes a single value
            ReduceExprByPrecedence(inner);

            /// Parantheses result is saved in the front of the inner vector.
            ExprItem it;
            it.value = inner.front().value;
            it.signage = inner.front().signage;
            it.floatness = inner.front().floatness;
            it.notCnt = pendingNotCnt;
            pendingNotCnt = 0;
            expr.push_back(it);
            continue;
        }
        // Closing parenthesis -> end recursion
        else if (token.GetValue() == ")")
        {
            break;
        }
        // Count 'not' for the next value
        else if (token.GetValue() == "not")
        {
            pendingNotCnt++;
            continue;
        }
        // HAS_PARAM -> function call or nested command (treated as single value)
        else if (token.GetType() == TokenTypes::HAS_PARAM)
        {
            if (paramIndex >= command.GetParams().size())
            {
                StoreInternalError("IRVisitor::CollectExprItems paramIndex overflow");
            }
            Command innerCommand = command.GetParams()[paramIndex++];

            IRBaseValue* currExpressionValue = nullptr;
            bool localSignage = false;
            bool localFloatness = false;

            if (innerCommand.GetType() == CommandTypes::IDENTIFIER_VALUE)
            {
                ASTNode tempNode(innerCommand);
                const std::optional<VarMetaData> varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(&tempNode));
                if (!varMetaData) return {};
                if (varMetaData->type == Type::UNSIGNED_INT_TYPE) localSignage = true;
                else if (varMetaData->type == Type::DOUBLE_TYPE) localFloatness = true;

                currExpressionValue = HandleIdentifierValue(&tempNode);
            }
            else if (innerCommand.GetType() == CommandTypes::FINISHED_FUNC_CALL)
            {
                FuncMetaData* funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(innerCommand[FUNC_IDENTIFIER_INDEX].GetValue());
                if (!funcMetaData) return {};
                if (funcMetaData->GetRType() == Type::UNSIGNED_INT_TYPE) localSignage = true;
                else if (funcMetaData->GetRType() == Type::DOUBLE_TYPE) localFloatness = true;

                currExpressionValue = HandleFuncCall(innerCommand);
            }
            else if (innerCommand.GetType() == CommandTypes::SIZEOF)
            {
                currExpressionValue = HandleSizeof(innerCommand);
            }
            else
            {
                StoreInternalError("IRVisitor::CollectExprItems unknown param");
            }

            ExprItem it;
            it.value = currExpressionValue;
            it.signage = localSignage;
            it.floatness = localFloatness;
            it.notCnt = pendingNotCnt;
            pendingNotCnt = 0;
            expr.push_back(it);
            continue;
        }
        // VALUE token 
        else if (token.GetType() == TokenTypes::VALUE)
        {
            bool localSignage = false;
            bool localFloatness = false;
            if (CheckSignage(token)) localSignage = true;
            if (CheckFloatness(token)) localFloatness = true;

            IRBaseValue* currExpressionValue = ValueToConstIR(token);

            ExprItem it;
            it.value = currExpressionValue;
            it.signage = localSignage;
            it.floatness = localFloatness;
            it.notCnt = pendingNotCnt;
            pendingNotCnt = 0;
            expr.push_back(it);
            continue;
        }
        // Operator token: attach operator to previous item as right operator
        else if (token.GetType() == TokenTypes::OPERATOR || token.GetType() == TokenTypes::KEYWORD)
        {
            Operators op = TokenizerOperatorToIROperator(token);

            if (expr.empty())
            {
                // Unary operator at expression start -> treat as (0 <op> x) for now.
                ExprItem zero;
                zero.value = this->m_irbuilder.GetInt32(0);
                zero.signage = false;
                zero.floatness = false;
                zero.notCnt = 0;
                zero.op = op;
                expr.push_back(zero);
            }
            else
            {
                // normal binary operator -> attach to previous item
                expr.back().op = op;
            }
            continue;
        }
        else
        {
            StoreInternalError("IRVisitor::CollectExprItems unknown expression token");
        }
    }

    return expr;
}

IRBaseValue* IRVisitor::HandleExpression(Command command)
{
    // quick-cases
    if (command.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        return HandleFuncCall(command);
    }
    else if (command.GetType() == CommandTypes::IDENTIFIER_VALUE)
    {
        ASTNode tempNode(command);
        return HandleIdentifierValue(&tempNode);
    }

    const std::vector<Token> tokens = command.GetTokens();
    size_t tokenIndex = 0;
    size_t paramIndex = 0;

    // Collapses all of the parantheses
    std::vector<ExprItem> expr = CollectExprItems(command, tokens, tokenIndex, paramIndex);
    if (expr.empty())
    {
        StoreInternalError("IRVisitor::HandleExpression somehow expression is null after flattening");
        return nullptr;
    }

    // Collapses by precedence (mul/div/rem before add/sub etc)
    ReduceExprByPrecedence(expr);

    // Finaly apply any leftover 'not' and return
    IRBaseValue* finalVal = ApplyNotIfNeeded(expr.front().value, expr.front().notCnt);
    return finalVal;
}


void IRVisitor::CheckValueNeededSomewhere(IRBaseValue* value, ASTNode* node)
{
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
    if (this->m_currFunc->GetBasicBlocks().back().get()->GetInstructions().size() == 0) return nullptr;
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
        StoreInternalError("IRVisitor::FindPointer Couldn't find stored var: " + name);
        return nullptr;
    }
}

void IRVisitor::StoreInternalError(const std::string& msg)
{
    this->m_errorManager.StoreError(CompileTimeError(ErrorTypes::INTERNAL_ERROR, msg, 0, 0, ""));
}
