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
        // auto & Blocks = CFGPtr->Blocks;
        // for (size_t i = 0; i < Blocks.size(); ++i) {
        //     auto & BlockPtr = Blocks[i];
        //     ss << "** Block[" << i << "]: " << jdc::ToString(BlockPtr) << "\n";
        // }

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
        assert(Blocks.empty());
        assert(BlockList.empty());

        InitLocalVariables();
        InitBlocks();
        X_DEBUG_PRINTF("xJavaControlFlowGraph InitBlocksDone:\n%s\n", ToString(this).c_str());

        // ReduceGoto(); in jd-core
        // ReduceLoop(); in jd-core
        // ReduceGraph();

        return true;
    }

    void xJavaControlFlowGraph::Clean()
    {
        xel::Renew(FirstVariableIndex);
        xel::Renew(LocalVariableList);
        xel::Renew(Blocks);
        xel::Renew(BlockList);

        _JavaClassPtr = nullptr;
        _JavaMethodPtr = nullptr;
    }

}
