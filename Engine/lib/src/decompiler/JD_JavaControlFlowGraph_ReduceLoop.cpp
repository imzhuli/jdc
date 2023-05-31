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
    [[maybe_unused]]
    static std::vector<bool> BuildDominatorIndexes(std::vector<xJavaBlock*> & BlockPtrList)
    {
        auto ArrayOfDominatorIndexes = std::vector<bool>();
        // List<BasicBlock> list = cfg.getBasicBlocks();
        // int length = list.size();
        // BitSet[] arrayOfDominatorIndexes = new BitSet[length];

        // BitSet initial = new BitSet(length);
        // initial.set(0);
        // arrayOfDominatorIndexes[0] = initial;

        // for (int i=0; i<length; i++) {
        //     initial = new BitSet(length);
        //     initial.flip(0, length);
        //     arrayOfDominatorIndexes[i] = initial;
        // }

        // initial = arrayOfDominatorIndexes[0];
        // initial.clear();
        // initial.set(0);

        // boolean change;

        // do {
        //     change = false;

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
        // } while (change);

        return ArrayOfDominatorIndexes;
    }


    void xJavaControlFlowGraph::ReduceLoop()
    {

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