#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>

namespace jdc
{


    std::unique_ptr<xJavaControlFlowGraph> xJavaControlFlowGraph::ParseByteCode(const xJavaClass * JavaClassPtr, const xAttributeCode & CodeAttribute)
    {
        auto JavaControlFlowGraphUPtr = std::make_unique<xJavaControlFlowGraph>();
        auto CFGPtr = JavaControlFlowGraphUPtr.get();





        (void) CFGPtr;
        return JavaControlFlowGraphUPtr;
    }

}
