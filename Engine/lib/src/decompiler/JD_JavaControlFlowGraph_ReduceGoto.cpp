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

    // public static void reduce(ControlFlowGraph cfg) {
    //     for (BasicBlock basicBlock : cfg.getBasicBlocks()) {
    //         if (basicBlock.getType() == TYPE_GOTO) {
    //             BasicBlock successor = basicBlock.getNext();

    //             if (basicBlock == successor) {
    //                 basicBlock.getPredecessors().remove(basicBlock);
    //                 basicBlock.setType(TYPE_INFINITE_GOTO);
    //             } else {
    //                 Set<BasicBlock> successorPredecessors = successor.getPredecessors();
    //                 successorPredecessors.remove(basicBlock);

    //                 for (BasicBlock predecessor : basicBlock.getPredecessors()) {
    //                     predecessor.replace(basicBlock, successor);
    //                     successorPredecessors.add(predecessor);
    //                 }

    //                 basicBlock.setType(TYPE_DELETED);
    //             }
    //         }
    //     }
    // }


    void xJavaControlFlowGraph::ReduceGoto()
    {
        auto Blocks = std::vector<xJavaBlock*>();
        Blocks.reserve(BlockList.size());
        for (auto & UPtr : BlockList) {
            Blocks.push_back(UPtr.get());
        }

        return;
    }













}