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

    void xJavaControlFlowGraph::ReduceGoto()
    {
        for (auto BlockPtr : BlockPtrList) {
            if (BlockPtr->Type != xJavaBlock::TYPE_GOTO) {
                continue;
            }

            auto SuccessorPtr = BlockPtr->NextBlockPtr;
            if (BlockPtr == SuccessorPtr) { // infinite goto
                BlockPtr->Predecessors.erase(BlockPtr->Predecessors.find(BlockPtr));
                BlockPtr->Type = xJavaBlock::TYPE_INFINITE_GOTO;
            }
            else {
                auto & SuccessorPredecessors = SuccessorPtr->Predecessors;
                SuccessorPredecessors.erase(SuccessorPredecessors.find(BlockPtr));
                for (auto & PredecessorPtr : BlockPtr->Predecessors) {
                    PredecessorPtr->Replace(BlockPtr, SuccessorPtr);
                    SuccessorPredecessors.insert(PredecessorPtr);
                }
                BlockPtr->Type = xJavaBlock::TYPE_DELETED;
            }
        }

        return;
    }













}