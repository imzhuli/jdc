#pragma once
#include "../base/JD_.hpp"
#include "../base/JD_Instructions.hpp"
#include "../class_file/JD_ClassFile.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <string>
#include <vector>

namespace jdc
{

    struct xJavaMethod
    {
        const xClassInfo *    ClassInfoPtr;
        const xMethodInfo *   MethodInfoPtr;

        // basic extraction
        std::string_view            OriginalNameView;
        std::string                 Identifier;
        xAttributeCode              AttributeCode;
        xAttributeMethodParameters  AttributeParameters;

        // decode
        std::string                       QualifierString;
        std::vector<std::string>          TypeBinaryNames;
        xAttributeLocalVariableTable      SubAttributeLocalVariableTable;
        xAttributeLocalVariableTypeTable  SubAttributeLocalVariableTypeTable;

        X_GAME_API_MEMBER void Decode();
        X_GAME_API_MEMBER void DecodeCodeAttributs();
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