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

    static std::vector<xBitSet> BuildDominatorIndexes(std::vector<xJavaBlock*> & BlockPtrList)
    {
        auto Length = BlockPtrList.size();
        auto ArrayOfDominatorIndexes = std::vector<xBitSet>(Length);
        ArrayOfDominatorIndexes[0] = xBitSet(Length);
        ArrayOfDominatorIndexes[0][0] = true;

        for (size_t i = 1; i < Length; ++i) {
            auto & Item = ArrayOfDominatorIndexes[i];
            Item = xBitSet(Length, true);
        }

        for (auto & Item : ArrayOfDominatorIndexes) {
            X_DEBUG_PRINTF("%s\n", ToString(Item).c_str());
        }

        auto Change = false;
        do {
            for (size_t Index = 1; Index < Length; ++Index) {
                auto BlockPtr = BlockPtrList[Index];
                assert(BlockPtr->Index == Index);

                auto & DominatorIndexes = ArrayOfDominatorIndexes[Index];
                auto   Backup = DominatorIndexes;
                for (auto PredecessorBlockPtr : BlockPtr->Predecessors) {
                    auto & PredecessorDominatorIndexes = ArrayOfDominatorIndexes[PredecessorBlockPtr->Index];
                    for (size_t i = 0; i < DominatorIndexes.size(); ++i) {
                        DominatorIndexes[i] = DominatorIndexes[i] & PredecessorDominatorIndexes[i];
                    }
                }
                DominatorIndexes[Index] = true;

                Change |= DominatorIndexes != Backup;
            }
        } while(Steal(Change));


        //     for (BasicBlock basicBlock : list) {
        //         int index = basicBlock.getIndex();
        //         BitSet dominatorIndexes = arrayOfDominatorIndexes[index];

        //         initial = (BitSet)dominatorIndexes.clone();

        //         for (BasicBlock predecessorBB : basicBlock.getPredecessors()) {
        //             dominatorIndexes.and(arrayOfDominatorIndexes[predecessorBB.getIndex()]);
        //         }

        //         dominatorIndexes.set(index);
        //         change |= (! initial.equals(dominatorIndexes));
        //     }

        for (auto & Item : ArrayOfDominatorIndexes) {
            X_DEBUG_PRINTF("%s\n", ToString(Item).c_str());
        }

        return ArrayOfDominatorIndexes;
    }


    void xJavaControlFlowGraph::ReduceLoop()
    {
        auto ArrayOfDominatorIndexes = BuildDominatorIndexes(BlockPtrList);
        // BitSet[] arrayOfDominatorIndexes = buildDominatorIndexes(cfg);
        // List<Loop> loops = identifyNaturalLoops(cfg, arrayOfDominatorIndexes);

        // for (int i=0, loopsLength=loops.size(); i<loopsLength; i++) {
        //     Loop loop = loops.get(i);
        //     BasicBlock startBB = loop.getStart();
        //     BasicBlock loopBB = reduceLoop(loop);

        //     // Update other loops
        //     for (int j=loopsLength-1; j>i; j--) {
        //         Loop otherLoop = loops.get(j);

        //         if (otherLoop.getStart() == startBB) {
        //             otherLoop.setStart(loopBB);
        //         }

        //         if (otherLoop.getMembers().contains(startBB)) {
        //             otherLoop.getMembers().removeAll(loop.getMembers());
        //             otherLoop.getMembers().add(loopBB);
        //         }

        //         if (otherLoop.getEnd() == startBB) {
        //             otherLoop.setEnd(loopBB);
        //         }
        //     }
        // }
        return;
    }













}