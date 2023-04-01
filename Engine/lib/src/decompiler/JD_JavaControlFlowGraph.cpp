#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>

namespace jdc
{


    std::unique_ptr<xJavaControlFlowGraph> xJavaControlFlowGraph::ParseByteCode(const xJavaMethod * JavaMethodPtr)
    {
        auto JavaControlFlowGraphUPtr = std::make_unique<xJavaControlFlowGraph>();
        auto CFGPtr = JavaControlFlowGraphUPtr.get();

        auto JavaClassPtr = JavaMethodPtr->JavaClassPtr;
        auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(JavaMethodPtr->Converted.AttributeMap, "Code");
        auto & CodeBinary = CodeAttributePtr->CodeBinary;


        (void) CodeBinary;
        (void) JavaClassPtr;
        (void) CFGPtr;

        return JavaControlFlowGraphUPtr;
    }

}
