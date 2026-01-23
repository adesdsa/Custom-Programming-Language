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
        m_currModule.AddVariable(varMetaData.sName, TypeToIRType(varMetaData.type));
    }
    else
    {
        m_irbuilder.CreateAlloca(TypeToIRType(varMetaData.type), varMetaData.sName);
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

    m_irbuilder.CreateLoad(IRBaseValue(varMetaData.sName, TypeToIRType(varMetaData.type)));
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
    
}

void IRVisitor::VisitFuncDefinition(ASTNode* node)
{
    return;
}

void IRVisitor::VisitFuncCall(ASTNode* node)
{

}

void IRVisitor::VisitIf(ASTNode* node)
{

}

void IRVisitor::VisitElif(ASTNode* node)
{

}

void IRVisitor::VisitElse(ASTNode* node)
{

}

void IRVisitor::VisitWhile(ASTNode* node)
{

}

void IRVisitor::VisitFor(ASTNode* node)
{

}

void IRVisitor::VisitReturn(ASTNode* node)
{
    if (m_currFunc.GetReturnType() == IRType::GetVoid())
    {
        m_irbuilder.CreateRetVoid();
    }
    else
    {
        
    }
}

void IRVisitor::VisitControlStatement(ASTNode* node)
{
    Token statement = node->command[0];

    if (statement.GetValue() == "continue")
    {

    }
    else if (statement.GetValue() == "break")
    {

    }
}

void IRVisitor::VisitBlockStart(ASTNode* node)
{

}

void IRVisitor::VisitBlockEnd(ASTNode* node)
{

}

void IRVisitor::VisitEndOfCommand(ASTNode* node)
{
    return;
}

void IRVisitor::VisitSizeof(ASTNode* node)
{

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
