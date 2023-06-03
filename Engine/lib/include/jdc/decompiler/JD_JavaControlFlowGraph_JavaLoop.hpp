#pragma once
#include "./_.hpp"
#include "./JD_JavaControlFlowGraph_JavaBlock.hpp"
#include "../base/JD_Instructions.hpp"
#include <memory>
#include <set>

namespace jdc
{
    class xJavaControlFlowGraph;
    class xJavaBlock;

    struct xJavaLoop
    {
        xJavaBlock *            StartBlockPtr;
        xJavaBlock *            EndBlockPtr;
        xJavaBlockPtrSet        MemberBlocks;

        /*
        * Smaller loop first
        */
        struct xComparator  {
            bool operator() (const xJavaLoop & lhs, const xJavaLoop & rhs) { // Compare
                return lhs.MemberBlocks.size() < rhs.MemberBlocks.size();
            }
        };
    };

    X_PRIVATE std::string ToString(const xJavaLoop & Loop);

}
