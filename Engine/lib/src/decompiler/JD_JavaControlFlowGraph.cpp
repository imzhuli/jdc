#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <xel/String.hpp>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace xel;

namespace jdc
{

    std::string ToString(const xJavaControlFlowGraph * CFGPtr)
    {
        auto ss = std::ostringstream();
        ss << "*************************\n";

        auto & BlockList = CFGPtr->BlockList;
        for (size_t i = 0; i < BlockList.size(); ++i) {
            auto & BlockUPtr = BlockList[i];
            ss << "Block[" << i << "]: " << ToString(BlockUPtr.get()) << "\n";
        }
        return ss.str();
    }

    xJavaControlFlowGraph::xJavaControlFlowGraph(const xJavaMethod * JavaMethodPtr)
    {
        _JavaMethodPtr = JavaMethodPtr;
        _JavaClassPtr = JavaMethodPtr->JavaClassPtr;

        EndBlockUPtr = std::make_unique<xJavaBlock>(this, xJavaBlock::TYPE_END);
        EndBlockPtr  = EndBlockUPtr.get();
    }

    std::unique_ptr<xJavaControlFlowGraph> xJavaControlFlowGraph::ParseByteCode(const xJavaMethod * JavaMethodPtr)
    {
        auto JavaControlFlowGraphUPtr = std::make_unique<xJavaControlFlowGraph>(JavaMethodPtr);
        auto CFGPtr = JavaControlFlowGraphUPtr.get();

        if (!CFGPtr->Init()) {
            return {};
        }
        return JavaControlFlowGraphUPtr;
    }

    bool xJavaControlFlowGraph::Init()
    {
        assert(LocalVariableList.empty());
        xel::Renew(FirstVariableIndex);
        assert(BlockList.empty());

        InitLocalVariables();
        InitBlocks();
        X_DEBUG_PRINTF("xJavaControlFlowGraph InitBlocks done:\n%s\n", ToString(this).c_str());

        ReduceGoto();
        X_DEBUG_PRINTF("xJavaControlFlowGraph ReduceGoto done:\n%s\n", ToString(this).c_str());

        ReduceLoop();
        X_DEBUG_PRINTF("xJavaControlFlowGraph ReduceLoop done:\n%s\n", ToString(this).c_str());

        // ReduceGraph();

        return true;
    }

    void xJavaControlFlowGraph::Clean()
    {
        xel::Renew(FirstVariableIndex);
        xel::Renew(LocalVariableList);
        xel::Renew(BlockList);

        _JavaClassPtr = nullptr;
        _JavaMethodPtr = nullptr;
    }

}
