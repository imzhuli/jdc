#pragma once
#include "./_.hpp"
#include <string>
#include <vector>
#include <memory>

namespace jdc
{
    class iJavaStatement;

    struct xJavaLocalVariable
    {
        std::string TypeCodeName;
        std::string VariableName;
    };

    class xJavaFrame
    {
    public:
        xJavaFrame *                                     ParentFramePtr;
        std::vector<xJavaLocalVariable>                  LocalVariableList;
        std::vector<std::unique_ptr<iJavaStatement>>     Statements;
        std::vector<std::unique_ptr<xJavaFrame>>         ChildFrames;
    };

}
