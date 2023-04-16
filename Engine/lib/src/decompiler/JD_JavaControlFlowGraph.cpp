#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <xel/String.hpp>

namespace jdc
{

    std::unique_ptr<xJavaControlFlowGraph> xJavaControlFlowGraph::ParseByteCode(const xJavaMethod * JavaMethodPtr)
    {
        auto JavaControlFlowGraphUPtr = std::make_unique<xJavaControlFlowGraph>();
        auto CFGPtr = JavaControlFlowGraphUPtr.get();

        if (!CFGPtr->Init(JavaMethodPtr)) {
            return {};
        }
        return JavaControlFlowGraphUPtr;
    }

    bool xJavaControlFlowGraph::Init(const xJavaMethod * JavaMethodPtr)
    {
        _JavaMethodPtr = JavaMethodPtr;
        _JavaClassPtr = JavaMethodPtr->JavaClassPtr;

        InitLocalVariables();
        InitBlocks();

        return true;
    }


    void xJavaControlFlowGraph::InitLocalVariables()
    {
        xel::Renew(LocalVariableList);
        FirstVariableIndex = 0;

        if (_JavaMethodPtr->MethodInfoPtr->AccessFlags & ACC_STATIC) {
            // no "this" parameter
        }
        else {
            LocalVariableList.push_back({ _JavaClassPtr->GetInnermostName(), "this"s });
            ++FirstVariableIndex;
        }

        if (_JavaMethodPtr->IsConstructor) {
            if (_JavaClassPtr->IsEnum()) {
                // TODO: enum constructor
            }
            else {
                if (_JavaClassPtr->IsInnerClass()) {
                    // add local variable this$0:
                    LocalVariableList.push_back({ _JavaClassPtr->Extend.OuterClassPtr->GetFixedCodeName(), "this$0"s });
                }
            }
        }

        X_DEBUG_PRINTF("JavaMethod: %s.%s local variables:\n", _JavaClassPtr->GetFixedCodeName().c_str(), _JavaMethodPtr->OriginalName.c_str());
        for (auto & Variable : LocalVariableList) {
            X_DEBUG_PRINTF("TypeCodeName: %s VariableName:%s\n", Variable.TypeCodeName.c_str(), Variable.VariableName.c_str());
        }
    }

    void xJavaControlFlowGraph::InitBlocks()
    {
        auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(_JavaMethodPtr->Converted.AttributeMap, "Code");
        auto & CodeBinary = CodeAttributePtr->CodeBinary;

        xel::Renew(BlockList);
        xel::Renew(_BlockPool);




        (void) CodeBinary;
    }

}
