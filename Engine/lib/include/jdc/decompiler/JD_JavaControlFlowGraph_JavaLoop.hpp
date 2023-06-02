#pragma once
#include "./_.hpp"
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
        std::set<xJavaBlock *>  MemberBlocks;

        /*
        * Smaller loop first
        */
        struct xComparator  {
            bool operator() (const xJavaLoop & lhs, const xJavaLoop & rhs) { // Compare
                return lhs.MemberBlocks.size() < rhs.MemberBlocks.size();
            }
        };
    };


}
