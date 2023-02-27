#pragma once
#include "../base/JD_Base.hpp"
#include "./JD_Instructions.hpp"
#include <string>
#include <vector>

namespace jdc
{

    struct xVariable
    {
        std::string  Name;
        const char * TypeBinaryName;
        uint32_t     LocalVariableIndex        = -1; // undetermined
        uint32_t     RuntimeStackIndex         = -1; // undetermined

        bool IsTempVariable() const { return RuntimeStackIndex != static_cast<uint32_t>(-1); }
    };

    struct xOpTreeNode
    {
        xOpCode                     OpCode;
        uint16_t                    OpInstructionLineIndex;

        xVariable                   Result;
        std::vector<xOpTreeNode*>   OperandNodes;
    };

    struct xOpInstrucionLine
    {
        size_t WhileEntryCount  = 0;
        size_t IfEntryCount     = 0;
        size_t ElseEntryCount   = 0;
        size_t SwitchEntryCount = 0;

        uint32_t LineIndex     = -1;
        bool     IsFlowControl = false;
    };

    struct xMethod
    {
        std::vector<xOpInstrucionLine>    InstructionLines;

        std::string Identifier;
        bool Synthetic = false;
    };

}
