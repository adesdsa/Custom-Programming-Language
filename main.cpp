#include <iostream>

#include "includes/utils/tests_utils.h"
#include "includes/utils/system_utils.h"
#include "includes/parser/parser_logic.h"
#include "includes/semantic_analysis/var_table_node.h"

namespace ReturnCode
{
    inline constexpr const int k_nSuccess = 0;
    inline constexpr const int k_nFailed = 1;
}

int main(void)
{
    int nReturnValue = ReturnCode::k_nSuccess;

    try
    {
        VarTableNode* n1 = new VarTableNode(nullptr);
        VarMetaData data1;
        data1.accessRights = AccessRights::Default;
        data1.offsetInsideFrame = 4;
        data1.scope = Scope::global;
        data1.sName = "x";
        data1.type = Type::Int;
        n1->InsertVar(data1);
        auto temp = n1->FindVar("x");
        if (temp)
        {
            std::cout << temp->sName << std::endl;
        }
        data1.sName = "y";
        n1->InsertVar(data1);
        std::cout << n1->GetStackOffset() << std::endl;
        VarTableNode* n2 = n1->CreateBlock();
        n1->CreateBlock();
        n2->CreateBlock();
        temp = n2->FindVar("x");
        if (temp)
        {
            std::cout << temp->sName << std::endl;
        }
        temp = n2->FindVar("y");
        if (temp)
        {
            std::cout << temp->sName << std::endl;
        }
        data1.sName = "z";
        n2->InsertVar(data1);
        std::cout << n2->GetStackOffset() << std::endl;
        //ParserLogic parser(SystemUtils::BuildRelativePath(L"tests/inputs/test4.txt"));
        //parser.MainFlow();
    }
    catch(const CompilationAbortException& abortError)
    {
        std::cerr << "ERROR: " << abortError.what() << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
    }
    catch(const std::runtime_error& runtimeError)
    {
        std::cerr << "Run time ERROR: " << runtimeError.what() << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
    }
    catch(const std::exception& generalError)
    {
        std::cerr << "General ERROR: " << generalError.what() << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
    }
    catch(...)
    {
        std::cerr << "Unknown ERROR" << std::endl;
        nReturnValue = ReturnCode::k_nFailed;
    }

    return nReturnValue;
}
