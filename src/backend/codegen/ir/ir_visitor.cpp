#include "includes/backend/codegen/ir/ir_visitor.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_string.h"

IRVisitor::IRVisitor(
    const EntityLogger& entityLogger,
    ErrorManager& errorManager,
    const DecoratedAST& decoratedAst,
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
    m_latchCnt(0),
    m_nextControlEntryBlock(nullptr),
    m_lastSavedValue(nullptr),
    m_currFunc(nullptr),
    m_currVarMetaData(std::nullopt),
    m_expectedReturnValue(false),
    m_expectedAssignment(false),
    m_returnedAlready(false),
    m_handledInsides(false),
    m_insideFor(false)
{
    this->InitializeHandlers();
    this->m_varTableIndex.push(0);
    this->m_currVarTableNode = &this->m_decoratedAst.varTable;

    // pre register all global variables in the IR module so they are
    // available before their declaration node is visited
    for (const auto& [name, varMeta] : this->m_decoratedAst.varTable.GetVars())
    {
        if (varMeta.scope == Scope::GLOBAL)
        {
            this->m_currModule.AddVariable(name, TypeToIRType(varMeta.type));
        }
        else if (varMeta.scope == Scope::EXPORT)
        {
            this->m_currModule.AddExport(name);
        }
        else if (varMeta.isExtern)
        {
            this->m_currModule.AddImport(name);
        }
    }

    for (const auto& [name, funcMetaData] : this->m_decoratedAst.funcTable.GetAllFunctions())
    {
        if (funcMetaData->IsExtern())
        {
            this->m_currModule.AddImport(name);
        }
        else
        {
            this->m_currModule.AddImport(name);
        }
    }
}

std::vector<std::unique_ptr<IRBaseValue>> IRVisitor::DumpConstants() noexcept
{
    auto tempConstHolder = this->m_irbuilder.DumpConstants();
    this->m_irbuilder.ClearConstants();
    return std::move(tempConstHolder);
}

void IRVisitor::VisitVarDeclaration(ASTNode* node)
{
    if (this->m_returnedAlready) return;
    if (this->m_handledInsides) return; // Incase already handled the insides.

    if (!this->m_currVarTableNode)
    {
        StoreInternalError("Variable declaration failed symbol table is unavailable", node->command);
        return;
    }
    
    /// Getting the varMetaData
    std::string varName = ASTExtractor::ExtractVarName(node);
    auto varMetaData = this->m_currVarTableNode->FindVar(varName);
    if (!varMetaData) 
    {
        return;
    }
    this->m_currVarMetaData = varMetaData; // Saving it incase there will be assigmnet later on

    // The var is saved differently if it is global or local.
    // variable declared at the top level (outside any function) is always
    // global
    if (varMetaData.value().scope == Scope::GLOBAL || !this->m_currFunc)
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
        // If entry block doesnt exist yet, this is a function parameter
        // parameters are already handled as IRArguments, no alloca needed
        if (this->m_currFunc->GetEntryBlock() == nullptr)
        {
            this->m_entityLogger.Info("Added VarDeclaration IR handling.\nAt line: "
                + std::to_string(node->command[0].GetLine()));
            return;  // Skip alloca for function parameters
        }
        
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
    this->m_entityLogger.Info("Added VarDeclaration IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitExpression(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    if (this->m_handledInsides) return; // Incase already handled the insides.
    IRBaseValue* expressionValue = HandleExpression(node->command);
    CheckValueNeededSomewhere(expressionValue, node);
    this->m_entityLogger.Info("Added Expression IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitIdentifier(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    // Getting the varMetaData and saving it for later use incase of assignment.
    std::optional<VarMetaData> varMetaData = this->m_currVarTableNode->FindVar(ASTExtractor::ExtractVarName(node));
    if (!varMetaData)
    {
        StoreInternalError("Unknown variable encountered", node->command);
        return;
    }
    this->m_currVarMetaData = varMetaData;
    this->m_entityLogger.Info("Added Identifier IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitIdentifierValue(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    if (this->m_handledInsides) return; // Incase already handled the insides.
    auto loadInstruction = HandleIdentifierValue(node);
    CheckValueNeededSomewhere(loadInstruction, node);
    this->m_entityLogger.Info("Added IdentifierValue IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitAssignment(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    if (this->m_insideFor) return; // Assignment inside for already handled.
    // If this flag is up it means the expression/func_call/idenitifierValue already finished.
    if (this->m_expectedAssignment)
    {
        IRBaseValue* valueToStore = this->m_lastSavedValue;

        // outside any function set the global variable's
        // initializer instead of emitting a runtime store instruction.
        if (!this->m_currFunc)
        {
            IRGlobalVariable* globalVar = this->m_currModule.GetVariable(
                this->m_currVarMetaData.value().sName);
            if (globalVar && valueToStore)
            {
                auto targetType = TypeToIRType(this->m_currVarMetaData.value().type);
                valueToStore = ConvertIfNeeded(valueToStore, targetType);
                globalVar->SetInitializer(valueToStore);
            }
            this->m_expectedAssignment = false;
            this->m_entityLogger.Info("Added Assignment IR handling.\nAt line: "
                + std::to_string(node->command[0].GetLine()));
            return;
        }

        IRBaseValue* target = FindPointer(this->m_currVarMetaData.value().sName);

        // Insert implicit type conversion when the value type differs from the target variable type
        if (valueToStore && target)
        {
            auto targetType = TypeToIRType(this->m_currVarMetaData.value().type);
            valueToStore = ConvertIfNeeded(valueToStore, targetType);
        }

        this->m_irbuilder.CreateStore(valueToStore, target);
        this->m_expectedAssignment = false;
        this->m_entityLogger.Info("Added Assignment IR handling.\nAt line: "
            + std::to_string(node->command[0].GetLine()));
    }
    else
    {
        this->m_expectedAssignment = true;
    }
}

void IRVisitor::VisitFuncDeclaration(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    
    // Check if function already exists in IR module (already declared)
    std::string funcName = node->command[FUNC_IDENTIFIER_INDEX].GetValue();
    if (this->m_currModule.IsFuncExistByName(funcName))
    {
        this->m_entityLogger.Info("Added FuncDeclaration IR handling.\nAt line: "
            + std::to_string(node->command[0].GetLine()));
        return;  // Function already declared, skip
    }
    
    // Chking if the func is already defined.
    FuncMetaData* funcMetaData = this->m_decoratedAst.funcTable.FindFunctionByName(funcName);
    if (!funcMetaData)
    {
        StoreInternalError("Unknown function encountered", node->command);
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
    this->m_entityLogger.Info("Added FuncDeclaration IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitFuncCall(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    if (this->m_handledInsides) return; // Incase already handled the insides.
    auto funcCallInstruction = HandleFuncCall(node->command);
    CheckValueNeededSomewhere(funcCallInstruction, node);
    this->m_entityLogger.Info("Added FuncCall IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    this->m_handledInsides = true;
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
    this->m_entityLogger.Info("Added If IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    this->m_handledInsides = true;
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
    this->m_entityLogger.Info("Added Elif IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    this->m_handledInsides = true;
}

void IRVisitor::VisitElse(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    this->m_blockHistory.push(BlockKind::elseBlock);
    this->m_nextExitBlocks.push(std::move(this->m_conditionMergeBlocks.top()));
    this->m_entityLogger.Info("Added Else IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
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
    // For while-loops, continue jumps directly to the condition (no increment latch)
    this->m_continueTargets.push(condBlock);
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
    this->m_entityLogger.Info("Added While IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    this->m_handledInsides = true;
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

    // Create the latch block for the increment (continue jumps here instead of for.cond)
    this->m_forLatchBlocks.push(std::make_unique<BasicBlock>(GetCorrespondingLabel(BlockKind::forBlock, EntranceKind::latch)));
    this->m_continueTargets.push(this->m_forLatchBlocks.top().get());

    {
        size_t currIdx = this->m_varTableIndex.top();
        this->m_varTableIndex.pop();
        this->m_varTableIndex.push(currIdx + 1);  // consumed one child
        VarTableNode* forInitScope = this->m_currVarTableNode->GetBlock(static_cast<int>(currIdx));
        if (forInitScope)
        {
            this->m_currVarTableNode = forInitScope;
        }
        this->m_varTableIndex.push(0);  // start indexing children of for-init scope
    }

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

    // Creating the condition expression inside the for.cond block
    HandleExpression(node->command.GetParams()[FOR_COND_INDEX]);
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
    this->m_entityLogger.Info("Added For IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    this->m_handledInsides = true;
    this->m_insideFor = true;
}

void IRVisitor::VisitReturn(ASTNode* node)
{
    // If we got the Instruction of the return expression
    if (this->m_expectedReturnValue)
    {
        this->m_expectedReturnValue = false;

        // convert return value to match the function's declared return type
        IRBaseValue* retVal = this->m_lastSavedValue;
        if (retVal && this->m_currFunc)
        {
            retVal = ConvertIfNeeded(retVal, this->m_currFunc->GetReturnType());
        }

        this->m_irbuilder.CreateRet(retVal);
        this->m_returnedAlready = true;  // Set AFTER return is complete
    }
    else if (m_currFunc->GetReturnType() == IRType::GetVoid())
    {
        this->m_irbuilder.CreateRetVoid();
        this->m_returnedAlready = true;  // Set AFTER return is complete
    }
    else
    {
        // Waiting for expression - don't set m_returnedAlready yet
        this->m_expectedReturnValue = true;
    }
    this->m_entityLogger.Info("Added Return IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitControlStatement(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    Token statement = node->command[STATEMENT_INDEX];

    if (statement.GetValue() == "continue")
    {
        /// while-loops: jumps to the condition block.
        /// for-loops: jumps to the latch block (increment then condition).
        this->m_irbuilder.CreateBr(this->m_continueTargets.top());
    }
    else if (statement.GetValue() == "break")
    {
        this->m_irbuilder.CreateBr(this->m_afterLoopBlocks.top());
    }
    this->m_returnedAlready = true;
    this->m_entityLogger.Info("Added ControlStatement IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitBlockStart(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    this->m_handledInsides = false; // Reset
    this->m_insideFor = false; // Reset

    if (this->m_blockHistory.empty())
    {
        this->m_entityLogger.Error("VisitBlockStart: m_blockHistory is empty!.\nAt line: "
            + std::to_string(node->command[0].GetLine()));
        return;
    }
    
    BlockKind currentBlock = this->m_blockHistory.top();
    
    // For functionBlock, varTable navigation already done in VisitFuncDefinition
    // (because params come before BLOCK_START but need function's scope)
    if (currentBlock != BlockKind::functionBlock)
    {
        // Moving the VarTable to the correct node.
        if (this->m_varTableIndex.empty())
        {
            this->m_entityLogger.Error("VisitBlockStart: m_varTableIndex is empty!.\nAt line: "
                + std::to_string(node->command[0].GetLine()));
            return;
        }
        size_t currVarTableIndex = this->m_varTableIndex.top();
        this->m_varTableIndex.pop();
        this->m_varTableIndex.push(currVarTableIndex + 1);
        
        if (this->m_currVarTableNode == nullptr)
        {
            this->m_entityLogger.Error("VisitBlockStart: m_currVarTableNode is null!.\nAt line: "
                + std::to_string(node->command[0].GetLine()));
            this->m_varTableIndex.push(0);
        }
        else
        {
            const auto& children = this->m_currVarTableNode->GetChildren();
            
            // Bounds check - some blocks may not have their own scope
            if (currVarTableIndex >= children.size())
            {
                // This block has no local var scope - that's ok for blocks without local vars
                this->m_varTableIndex.push(0);
            }
            else
            {
                VarTableNode* nextNode = this->m_currVarTableNode->GetBlock(static_cast<int>(currVarTableIndex));
                if (nextNode != nullptr)
                {
                    this->m_currVarTableNode = nextNode;
                }
                this->m_varTableIndex.push(0);
            }
        }
    }

    switch (this->m_blockHistory.top())
    {
    case BlockKind::functionBlock:
    {
        this->m_irbuilder.SetInsertPoint(this->m_currFunc->AddBasicBlock("entry"));
        // global parameters, store the argument value into the global variable
        FuncMetaData* funcMeta = this->m_decoratedAst.funcTable.FindFunctionByName(
            this->m_currFunc->GetName());
        if (funcMeta)
        {
            for (const auto& param : funcMeta->GetFuncParams())
            {
                if (param.scope == Scope::GLOBAL)
                {
                    IRBaseValue* arg = this->m_currFunc->GetArgument(param.sName);
                    IRBaseValue* globalVar = this->m_currModule.GetVariable(param.sName);
                    if (arg && globalVar)
                    {
                        this->m_irbuilder.CreateStore(arg, globalVar);
                    }
                }
            }
        }
        break;
    }

    case BlockKind::elseBlock:
        break;

    case BlockKind::forBlock:
    case BlockKind::whileBlock:
    case BlockKind::elifBlock:
    case BlockKind::ifBlock:
        if (!this->m_currFunc || !this->m_nextControlEntryBlock)
        {
            this->m_entityLogger.Error("VisitBlockStart: m_currFunc or m_nextControlEntryBlock is null!.\nAt line: "
                + std::to_string(node->command[0].GetLine()));
            break;
        }
        
        // Setting the entry block as the current one.
        this->m_irbuilder.SetInsertPoint(
            this->m_currFunc->AddBasicBlock(
                std::move(this->m_nextControlEntryBlock)
            )
        );
        this->m_nextControlEntryBlock = nullptr;
        break;
    
    default:
        break;
    }

    VisitEndOfCommand(nullptr); // Resets
    this->m_entityLogger.Info("Added BlockStart IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
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
    std::vector<Command> tempParams;

    // Moving the VarTable to the correct node.
    this->m_varTableIndex.pop();
    this->m_currVarTableNode = this->m_currVarTableNode->GetParent();

    switch (this->m_blockHistory.top())
    {
    case BlockKind::functionBlock:
    {
        if (this->m_currFunc->GetReturnType() == IRType::GetVoid())
        {
            this->m_irbuilder.CreateRetVoid();
        }
        /// ====== Resets ======
        this->m_currFunc = nullptr;
        this->m_irbuilder.SetInsertPoint(nullptr);

        this->m_thenCnt = 0;
        this->m_elseCnt = 0;
        this->m_condCnt = 0;
        this->m_bodyCnt = 0;
        this->m_exitCnt = 0;
        this->m_mergeCnt = 0;
        this->m_latchCnt = 0;

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
        while (!this->m_continueTargets.empty())
        {
            this->m_continueTargets.pop();
        }
        while (!this->m_forLatchBlocks.empty())
        {
            this->m_forLatchBlocks.pop();
        }
        
        // Save the top-level function index before clearing (tells us which function we're at)
        size_t nextFuncIndex = this->m_varTableIndex.top();  // This is already incremented
        while (!this->m_varTableIndex.empty())
        {
            this->m_varTableIndex.pop();
        }
        this->m_nextControlEntryBlock = nullptr;
        
        // Re-initialize state for next function
        this->m_currVarTableNode = &this->m_decoratedAst.varTable;  // Reset to root
        this->m_varTableIndex.push(nextFuncIndex);  // Preserve index for next function
        this->m_entityLogger.Info("Added BlockEnd IR handling.\nAt line: "
            + std::to_string(node->command[0].GetLine()));
        return;  // Early return - blockHistory already emptied
    }

    case BlockKind::forBlock:
    {
        forNode = this->m_forIncrements.top();
        this->m_forIncrements.pop();

        // If the body is not already terminated (by break/return), branch to the latch block.
        BasicBlock* bodyInsertBlk = this->m_irbuilder.GetInsertBlock();
        if (bodyInsertBlk && !bodyInsertBlk->HasTerminator())
        {
            this->m_irbuilder.CreateBr(this->m_forLatchBlocks.top().get());
        }
        BasicBlock* latchBlk = this->m_currFunc->AddBasicBlock(std::move(this->m_forLatchBlocks.top()));
        this->m_forLatchBlocks.pop();
        this->m_irbuilder.SetInsertPoint(latchBlk);

        forCommand = forNode->command;
        tempParams = forCommand.GetParams();
        tempNode = (forCommand.GetParams()[FOR_PRE_AFTER_COND_VAR_INDEX]);
        VisitIdentifier(&tempNode);
        tempNode = ASTNode(forCommand.GetParams()[FOR_PRE_AFTER_COND_ASSIGNMENT_INDEX]);
        VisitAssignment(&tempNode);
        tempNode = ASTNode(forCommand.GetParams()[FOR_PRE_AFTER_COND_EXPRESSION_INDEX]);
        VisitExpression(&tempNode);

        // Branch from latch back to the condition block
        this->m_irbuilder.CreateBr(this->m_conditionBlocks.top());

        this->m_varTableIndex.pop();
        this->m_currVarTableNode = this->m_currVarTableNode->GetParent();

        this->m_continueTargets.pop();

        this->m_conditionBlocks.pop();
        this->m_afterLoopBlocks.pop();

        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        break;
    }

    case BlockKind::whileBlock:
    {
        BasicBlock* loopInsertBlk = this->m_irbuilder.GetInsertBlock();
        if (loopInsertBlk && !loopInsertBlk->HasTerminator())
        {
            this->m_irbuilder.CreateBr(this->m_conditionBlocks.top());
        }

        this->m_continueTargets.pop();

        this->m_conditionBlocks.pop();
        this->m_afterLoopBlocks.pop();

        // Creating the loop exit block
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        break;
    }

    case BlockKind::ifBlock:
    case BlockKind::elifBlock:
    {
        BasicBlock* ifInsertBlk = this->m_irbuilder.GetInsertBlock();
        if (ifInsertBlk && !ifInsertBlk->HasTerminator())
        {
            this->m_irbuilder.CreateBr(this->m_conditionMergeBlocks.top().get());
        }
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);

        // If this is last if or elif merge block is supposed to be here.
        if (node->nodeNextSibling == nullptr ||
            (node->nodeNextSibling->command.GetType() != CommandTypes::IF &&
            node->nodeNextSibling->command.GetType() != CommandTypes::ELIF &&
            node->nodeNextSibling->command.GetType() != CommandTypes::ELSE))
        {
            tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_conditionMergeBlocks.top()));
            this->m_conditionMergeBlocks.pop();
            this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        }
        break;
    }

    case BlockKind::elseBlock:
    {
        BasicBlock* elseInsertBlk = this->m_irbuilder.GetInsertBlock();
        if (elseInsertBlk && !elseInsertBlk->HasTerminator())
        {
            this->m_irbuilder.CreateBr(this->m_nextExitBlocks.top().get());
        }
        tempExitBlockHolder = this->m_currFunc->AddBasicBlock(std::move(this->m_nextExitBlocks.top()));
        this->m_nextExitBlocks.pop();
        this->m_conditionMergeBlocks.pop();
        this->m_irbuilder.SetInsertPoint(tempExitBlockHolder);
        break;
    }
    
    default:
        break;
    }

    VisitEndOfCommand(nullptr); // Resets
    this->m_blockHistory.pop(); // Reset
    this->m_entityLogger.Info("Added BlockEnd IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitSizeof(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    if (this->m_handledInsides) return; // Incase already handled the insides.
    auto tempSizeofInstruction = HandleSizeof(node->command);
    CheckValueNeededSomewhere(tempSizeofInstruction, node);
    this->m_entityLogger.Info("Added Sizeof IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    this->m_handledInsides = true;
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
        StoreInternalError("Unknown function encountered", node->command);
        return;
    }
    this->m_currFunc = foundFunc;
    this->m_blockHistory.push(BlockKind::functionBlock);
    
    // Navigate to function's scope in varTable BEFORE params are visited
    // (params come before BLOCK_START in AST but belong to function's scope)
    size_t currVarTableIndex = this->m_varTableIndex.top();
    this->m_varTableIndex.pop();
    this->m_varTableIndex.push(currVarTableIndex + 1);
    
    VarTableNode* nextNode = this->m_currVarTableNode->GetBlock(static_cast<int>(currVarTableIndex));
    if (nextNode != nullptr)
    {
        this->m_currVarTableNode = nextNode;
    }
    this->m_varTableIndex.push(0);
    
    this->m_entityLogger.Info("Added FuncDefinition IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
}

void IRVisitor::VisitEndOfCommand(ASTNode* node)
{
    if (this->m_returnedAlready) return; // Incase already returned
    // Only resetting.
    this->m_lastSavedValue = nullptr;
    this->m_handledInsides = false;
    this->m_currVarMetaData = std::nullopt;
    this->m_expectedReturnValue = false;
    this->m_expectedAssignment = false;
    if (node) // If this is null it was called from blockEnd
    {
        this->m_entityLogger.Info("Added EndOfCommand IR handling.\nAt line: "
            + std::to_string(node->command[0].GetLine()));
    }
}

void IRVisitor::VisitVoidCommand(ASTNode* node)
{
    this->m_entityLogger.Info("Added VoidCommand IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    return;
}

void IRVisitor::VisitPass(ASTNode* node)
{
    this->m_entityLogger.Info("Added Pass IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
    return;
}

void IRVisitor::VisitImportStatement(ASTNode* node)
{
    // Function does nothing
    return;
}

void IRVisitor::VisitEof(ASTNode* node)
{
    this->m_entityLogger.Info("Added Eof IR handling.\nAt line: "
        + std::to_string(node->command[0].GetLine()));
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
        StoreInternalError("Unsupported or invalid type encountered", Command());
        return IRType::GetVoid();
        break;
    }
}

std::shared_ptr<const IRType> IRVisitor::TypeToIRType(const std::string& type)
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

    StoreInternalError("Unsupported or invalid type encountered", Command());
    return IRType::GetVoid();
}

IRBaseValue* IRVisitor::ValueToConstIR(Token token)
{
    std::string value = token.GetValue();

    if (token.GetType() == TokenTypes::STRING_LITERAL)
    {
        // remove quotes
        std::string content = value.substr(1, value.size() - 2);

        // process escape sequences (\n, \t)
        std::string processed;
        for (size_t i = 0; i < content.size(); i++)
        {
            if (content[i] == '\\' && i + 1 < content.size())
            {
                switch (content[i + 1])
                {
                    case 'n':  
                        processed += '\n'; 
                        i++;
                        break;

                    case 't':  
                        processed += '\t';
                        i++;
                        break;
                        
                    case '\\':
                        processed += '\\';
                        i++;
                        break;
                        
                    case '"':  
                        processed += '"';
                        i++;
                        break;

                    default:  
                        processed += content[i];
                        break;
                }
            }
            else
            {
                processed += content[i];
            }
        }

        return this->m_irbuilder.GetString(processed);
    }

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

    case Operators::FloorDiv:
        if (signage)
        {
            return this->m_irbuilder.CreateSFloorDiv(lsh, rsh);
        }
        else if (floatness)
        {
            return this->m_irbuilder.CreateFFloorDiv(lsh, rsh);
        }
        else
        {
            return this->m_irbuilder.CreateUFloorDiv(lsh, rsh);
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
        StoreInternalError("Unsupported operator encountered", Command());
        return nullptr;
        break;
    }
}

Operators IRVisitor::TokenizerOperatorToIROperator(Token token)
{
    std::string value = token.GetValue();
    if (value == "and")
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
        return Operators::FloorDiv;
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
    else if (value == ">=")
    {
        return Operators::Ge;
    }
    else if (value == "<=")
    {
        return Operators::Le;
    }
    else if (value == "!=")
    {
        return Operators::Ne;
    }
    else return Operators::None;
}

bool IRVisitor::CheckSignage(Token token)
{
    // integer literals are always treated as signed
    // unless variables explicitly declared as unsigned
    (void)token;
    return false;
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
        StoreInternalError("Unknown variable encountered while evaluating expression", node->command);
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
        StoreInternalError("Unknown function encountered", command);
        return nullptr;
    }
    const std::vector<VarMetaData>& params = funcMetaData->GetFuncParams();

    // For built-in functions (e.g. println), lazily add a stub
    // IRFunction to the module so that CreateCall has a target.
    const std::string& funcName = funcMetaData->GetName();
    if (!this->m_currModule.IsFuncExistByName(funcName))
    {
        this->m_currModule.AddFunction(funcName, TypeToIRType(funcMetaData->GetRType()));
    }

    // Setting up the temporary holder
    std::vector<IRBaseValue*> tempInstructionHolder;

    // Only if the func has arguements we add them
    if (params.size() != 0 && params[0].type != Type::VOID_TYPE)
    {
        const auto& argExprs = command.GetParams();
        for (size_t i = 0; i < argExprs.size(); i++)
        {
            IRBaseValue* argValue = HandleExpression(argExprs[i]);

            // For println, skip implicit type conversion because we
            // dispatch to a specialized variant based on the original
            // argument type (println_double, println_bool, println_str).
            if (argValue && i < params.size() && funcName != "println")
            {
                auto expectedType = TypeToIRType(params[i].type);
                argValue = ConvertIfNeeded(argValue, expectedType);
            }

            tempInstructionHolder.push_back(argValue);
        }
    }

    std::string targetFuncName = funcName;
    if (funcName == "println" && !tempInstructionHolder.empty() && tempInstructionHolder[0])
    {
        auto argKind = tempInstructionHolder[0]->GetType()->GetKind();
        if (argKind == IRTypeKind::Float64 || argKind == IRTypeKind::Float32)
        {
            targetFuncName = "println_double";
        }
        else if (argKind == IRTypeKind::Int1)
        {
            targetFuncName = "println_bool";
        }
        else if (dynamic_cast<IRConstantString*>(tempInstructionHolder[0]))
        {
            targetFuncName = "println_str";
        }
    }

    // ensure the target function stub exists in the module
    if (targetFuncName != funcName && !this->m_currModule.IsFuncExistByName(targetFuncName))
    {
        this->m_currModule.AddFunction(targetFuncName, IRType::GetVoid());
    }

    // Creating the Call Instruction
    IRBaseValue* funcCallValue = this->m_irbuilder.CreateCall(
        this->m_currModule.GetFunction(targetFuncName),
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
        StoreInternalError("sizeof: could not determine the type", command);
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

        case EntranceKind::latch:
            this->m_latchCnt++;
            return "for.latch" + std::to_string(this->m_latchCnt - 1);
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

    this->m_mergeCnt++;
    std::string tempLabelHolder = "merge" + std::to_string(this->m_mergeCnt - 1);
    StoreInternalError("Could not create expected label using default: " + tempLabelHolder, Command());
    return tempLabelHolder;
}

bool IRVisitor::IsOneOf(Operators op, std::initializer_list<Operators> set)
{
    for (Operators currCheck : set) if (op == currCheck) return true;
    return false;
}

IRBaseValue* IRVisitor::ApplyNotIfNeeded(IRBaseValue* value, size_t notCnt)
{
    if (!value)
    {
        StoreInternalError("Null value encountered when applying 'not' operator", Command());
        return nullptr;
    }
    if (notCnt % 2 == 0) return value;
    // Logical NOT: value == 0  →  1, value != 0  →  0
    // Works for both booleans (0/1) and integers (0 is false, nonzero is true).
    return this->m_irbuilder.CreateICmpEQ(value, this->m_irbuilder.GetInt32(0));
}

IRBaseValue* IRVisitor::ConvertIfNeeded(IRBaseValue* value, std::shared_ptr<const IRType> targetType)
{
    if (!value || !targetType) return value;

    auto srcKind = value->GetType()->GetKind();
    auto dstKind = targetType->GetKind();

    if (srcKind == dstKind) return value; // no conversion needed

    bool srcIsFloat = (srcKind == IRTypeKind::Float32 || srcKind == IRTypeKind::Float64);
    bool dstIsFloat = (dstKind == IRTypeKind::Float32 || dstKind == IRTypeKind::Float64);
    bool srcIsInt   = (srcKind == IRTypeKind::Int1 || srcKind == IRTypeKind::Int8 ||
                       srcKind == IRTypeKind::Int32 || srcKind == IRTypeKind::Int64);
    bool dstIsInt   = (dstKind == IRTypeKind::Int1 || dstKind == IRTypeKind::Int8 ||
                       dstKind == IRTypeKind::Int32 || dstKind == IRTypeKind::Int64);

    if (srcIsFloat && dstIsInt)
    {
        return this->m_irbuilder.CreateFPToSI(value, targetType);
    }
    else if (srcIsInt && dstIsFloat)
    {
        return this->m_irbuilder.CreateSIToFP(value, targetType);
    }

    return value; // same category, no conversion (e.g. Int32 → Int64 handled by mov)
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

        if (!result)
        {
            this->m_entityLogger.Error("ReduceExprByOperators: ChooseOperation returned nullptr for op " + 
                                       std::to_string(static_cast<int>(currOp)));
            ++i;
            continue;
        }

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
    ReduceExprByOperators(expr, { Operators::Mul, Operators::Div, Operators::Rem, Operators::FloorDiv });
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
                StoreInternalError("Expression has more parameters than expected", command);
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
                StoreInternalError("Unknown parameter type in expression", command);
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
        else if (token.GetType() == TokenTypes::STRING_LITERAL)
        {
            IRBaseValue* currExpressionValue = ValueToConstIR(token);

            ExprItem it;
            it.value = currExpressionValue;
            it.signage = false;
            it.floatness = false;
            it.notCnt = 0;
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
            StoreInternalError("Unknown token in expression", command);
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
    else if (command.GetType() == CommandTypes::SIZEOF)
    {
        return HandleSizeof(command);
    }

    const std::vector<Token> tokens = command.GetTokens();
    size_t tokenIndex = 0;
    size_t paramIndex = 0;

    // Collapses all of the parantheses
    std::vector<ExprItem> expr = CollectExprItems(command, tokens, tokenIndex, paramIndex);
    if (expr.empty())
    {
        StoreInternalError("Expression evaluated to null could not generate code for this expression", command);
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
    IRBaseValue* searchVar = this->m_currFunc ? this->m_currFunc->FindAllocatedVar(name) : nullptr;
    auto searchGlobal = this->m_currModule.GetVariable(name);
    IRBaseValue* searchArg = this->m_currFunc ? this->m_currFunc->GetArgument(name) : nullptr;
    if (searchVar)
    {
        return searchVar;
    }
    else if (searchGlobal)
    {
        return searchGlobal;
    }
    else if (searchArg)
    {
        return searchArg;
    }
    else
    {
        StoreInternalError("Variable '" + name + "' not found", Command());
        return nullptr;
    }
}

void IRVisitor::StoreInternalError(const std::string& msg, const Command& command)
{
    if (command.GetTokens().size() == 0)
    {
        this->m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            msg,
            0,
            0,
            "")
        );
    }
    else
    {
        this->m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            msg,
            command[0].GetLine(),
            command[0].GetColumn(),
            "")
        );
    }
}
