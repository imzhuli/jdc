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

        auto JavaClassPtr = JavaMethodPtr->JavaClassPtr;
        auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(JavaMethodPtr->Converted.AttributeMap, "Code");
        auto & CodeBinary = CodeAttributePtr->CodeBinary;

        size_t FirstVariableIndex = 0;

        if (JavaMethodPtr->MethodInfoPtr->AccessFlags & ACC_STATIC) {
            // no "this" parameter
        }
        else {
            CFGPtr->LocalVariableList.push_back({ JavaClassPtr->GetInnermostName(), "this"s });
            ++FirstVariableIndex;
        }

        if (JavaMethodPtr->IsConstructor) {
            if (JavaClassPtr->IsEnum()) {
                // TODO: enum constructor
            }
            else {
                if (JavaClassPtr->IsInnerClass()) {
                    // add local variable this$0:
                    CFGPtr->LocalVariableList.push_back({ JavaClassPtr->Extend.OuterClassPtr->GetFixedCodeName(), "this$0"s });
                }
            }
        }

        X_DEBUG_PRINTF("JavaMethod: %s.%s local variables:\n", JavaClassPtr->GetFixedCodeName().c_str(), JavaMethodPtr->OriginalName.c_str());
        for (auto & Variable : CFGPtr->LocalVariableList) {
            X_DEBUG_PRINTF("TypeCodeName: %s VariableName:%s\n", Variable.TypeCodeName.c_str(), Variable.VariableName.c_str());
        }


        (void) CodeBinary;
        (void) JavaClassPtr;
        (void) CFGPtr;

        return JavaControlFlowGraphUPtr;
    }

}
