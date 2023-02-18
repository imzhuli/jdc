#pragma once
#include "../base/JD_Base.hpp"
#include "./JD_Instructions.hpp"
#include <string>
#include <vector>

namespace jdc
{

    struct xMethod
    {
        std::string ResultTypeBinaryName;
        std::vector<std::string> ArgumentTypeBinaryName;
    };

    struct xOperand
    {
        std::string Name;
        std::string TypeBinaryName;
        uint16_t    LocalVariableIndex        = -1; // undetermined
        uint16_t    RuntimeStackIndex         = -1; // undetermined

        bool IsTempVariable() const { return RuntimeStackIndex != -1; }
    };

    struct xOperationTreeNode
    {
        xOpCode OpCode;
        std::vector<xOperationTreeNode*> OperandNodes;
    };


}
