#pragma once
#include "../base/JD_Base.hpp"
#include "../base/JD_Instructions.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include <xel/View.hpp>
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

    struct xJavaMethod
    {
        const xClassInfo *    ClassInfoPtr;
        const xMethodInfo *   MethodInfoPtr;

        // basic extraction
        std::string_view  OriginalNameView;
        std::string       Identifier;

        // decode
        std::string                       QualifierString;
        std::vector<std::string>          TypeBinaryNames;
        xel::xDataView                    CodeBinaryView;
        std::vector<xOpInstrucionLine>    InstructionLines;

        X_GAME_API_MEMBER void Decode();
        X_GAME_API_MEMBER void DecodeNameStrings();
        X_GAME_API_MEMBER void Decode_Round_1();

        // only after decoding, the following have meanings
        X_INLINE size_t GetParamNumber() const { return TypeBinaryNames.size() - 1; }
        X_INLINE const std::string & GetParamTypeBinaryName(size_t Index) const { return TypeBinaryNames[Index]; }
        X_INLINE const std::string & GetReturnTypeBinaryName() const { return TypeBinaryNames[GetParamNumber()]; }
        X_GAME_API_MEMBER std::string GetQualifiedName();
        X_GAME_API_MEMBER std::string GetUnqualifiedName();
    };

}
