#include <jdc/decompiler/_.hpp>
#include <sstream>

using namespace xel;
using namespace std;

namespace jdc
{

    std::string ToString(const xBitSet & BitSet)
    {
        auto OS = std::ostringstream();
        OS << '{';
        bool First = true;
        for (size_t Index = 0 ; Index < BitSet.size() ; ++Index) {
            if (BitSet[Index]) {
                if (Steal(First)) {
                    OS << Index;
                } else {
                    OS << ", " << Index;
                }
            }
        }
        OS << '}';
        return OS.str();
    }

}
