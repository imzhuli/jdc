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

    void xJavaControlFlowGraph::InitLocalVariables()
    {
        xel::Renew(ClassNameBlackList);
        xel::Renew(LocalVariableList);
        xel::Renew(LocalVariablePtrList);
        size_t FirstVariableIndex = 0;

        if (_JavaMethodPtr->MethodInfoPtr->AccessFlags & ACC_STATIC) { // no "this" parameter
        }
        else {
            // TODO:
            ++FirstVariableIndex;
        }

        if (_JavaMethodPtr->IsConstructor) {
            if (_JavaClassPtr->IsEnum()) {
                // TODO: enum constructor
            }
            else {
                if (_JavaClassPtr->IsInnerClass()) {
                    // TODO: add local variable this$0:
                }
            }
        }

        X_DEBUG_PRINTF("JavaMethod: %s.%s local variables:\n", _JavaClassPtr->GetFixedCodeName().c_str(), _JavaMethodPtr->OriginalName.c_str());
        for (auto & VariablePtr : LocalVariablePtrList) {
            X_DEBUG_PRINTF("VariableName:%s\n", VariablePtr->GetName().c_str());
        }
    }

    void xJavaControlFlowGraph::CleanLocalVariables()
    {
        xel::Renew(LocalVariablePtrList);
        xel::Renew(LocalVariableList);
        xel::Renew(ClassNameBlackList);
    }

}
