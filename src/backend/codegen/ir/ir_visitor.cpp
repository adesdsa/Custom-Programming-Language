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
        IRBaseValue(
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
    /// @todo phi handling + control statement related
}

void IRVisitor::VisitBlockEnd(ASTNode* node)
{
    /// @todo phi handling + control statement related
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
std::shared_ptr<const IRType> TypeToIRType(Type type)
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
