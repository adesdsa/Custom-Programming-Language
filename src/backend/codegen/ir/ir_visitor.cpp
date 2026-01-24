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
    size_t paramIndex = 0;
    size_t notCnt = 0;
    IRInstruction* lastValue = nullptr;
    Operators lastOperator = Operators::None;
    bool signage = false;
    bool floatness = false;

    for (auto token : node->command.GetTokens())
    {
        if (token.GetType() == TokenTypes::HAS_PARAM)
        {
            Command innerCommand = node->command[paramIndex];
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
    std::string varName = ASTExtractor::ExtractVarName(node);
    const VarMetaData& varMetaData = m_decoratedAst.varTable.FindVar(varName);
    if (!varMetaData) return;
    m_currVarMetaData = varMetaData;

    m_irbuilder.CreateLoad(
        IRBaseValue( /// @todo fix this
            varMetaData.sName,
            TypeToIRType(
                varMetaData.type
            )
        )
    );
}

void IRVisitor::VisitAssignment(ASTNode* node)
{
    m_irbuilder.CreateStore(
        IRBaseValue(
            m_lastSavedInstruction->GetName(),
            m_lastSavedInstruction->GetType()
        ), 
        IRBaseValue(
            varMetaData.sName,
            TypeToIRType(varMetaData.type)
        )
    );
}

void IRVisitor::VisitFuncDeclaration(ASTNode* node)
{
    /// @todo save the args + more + create block
}

void IRVisitor::VisitFuncDefinition(ASTNode* node)
{
    return;
}

void IRVisitor::VisitFuncCall(ASTNode* node)
{
    /// @todo save the returned value of the func
}

void IRVisitor::VisitIf(ASTNode* node)
{
    /// @todo phi handling + create block
}

void IRVisitor::VisitElif(ASTNode* node)
{
    /// @todo phi handling + create block
}

void IRVisitor::VisitElse(ASTNode* node)
{
    /// @todo phi handling + create block
}

void IRVisitor::VisitWhile(ASTNode* node)
{
    /// @todo create block
}

void IRVisitor::VisitFor(ASTNode* node)
{
    /// @todo create block
}

void IRVisitor::VisitReturn(ASTNode* node)
{
    if (m_expectedReturnValue)
    {
        m_expectedReturnValue = false;

        m_irbuilder.CreateRet(
            IRBaseValue(
                m_lastSavedInstruction->GetName(),
                m_lastSavedInstruction->GetType()
            )
        );
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
        /// @todo will need to jump back to the start of the block
    }
    else if (statement.GetValue() == "break")
    {
        /// @todo will need to find the end of the block and jump to it
    }
}

void IRVisitor::VisitBlockStart(ASTNode* node)
{
    /// @todo phi handling + control statement related + move in the varTable
}

void IRVisitor::VisitBlockEnd(ASTNode* node)
{
    /// @todo phi handling + control statement related + move in the varTable
}

void IRVisitor::VisitEndOfCommand(ASTNode* node)
{
    return;
}

void IRVisitor::VisitSizeof(ASTNode* node)
{
    const Command& SizeofArg = node->command.GetParams()[0];
    if (SizeofArg.GetType() == CommandTypes::EXPRESSION)
    {
        /// @todo first need to wait until we will get the value of the expression inside
    }
    else // Sizeof with type
    {
        /// @todo here save a constant value
    }
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
