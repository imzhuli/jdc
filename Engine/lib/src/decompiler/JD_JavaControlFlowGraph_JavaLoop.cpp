#include <jdc/decompiler/JD_JavaControlFlowGraph_JavaLoop.hpp>
#include <sstream>

using namespace xel;
using namespace std;

namespace jdc
{

    std::string ToString(const xJavaLoop & Loop)
    {
        auto OS = std::ostringstream();
        OS << "Loop{start=" << Loop.StartBlockPtr->Index << ", members=[";

        bool First = true;
        for (auto & MemberBlockPtr : Loop.MemberBlocks) {
            if (Steal(First)) {
                OS << MemberBlockPtr->Index;
            }
            else {
                OS << ", " << MemberBlockPtr->Index;
            }
        }
        OS << "], end=" ;
        if (Loop.EndBlockPtr) {
            OS << Loop.EndBlockPtr->Index;
        }
        OS << "}";
        return OS.str();
    }

}
