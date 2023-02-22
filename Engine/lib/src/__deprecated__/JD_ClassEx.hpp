#pragma once
#include "./JD_Class.hpp"
#include <xel/Byte.hpp>

namespace jdc
{

    struct xClassEx
    {
        std::string                        SourceFile;
        std::string                        FullClassName;
        std::string                        FullSuperClassName;
        std::vector<std::string>           InterfaceNames;
        std::vector<xInnerClassAttribute>  InnerClasses;
    };

    struct xFieldEx
    {
        std::string        Name;
        xAccessFlag        AccessFlags;
        xVariableType      Type;
        std::string        TypeString;
        std::string        InitValueString;
    };

    struct xMethodEx
    {
        std::string                ClassName;
        std::string                Name;
        xAccessFlag                AccessFlags;
        std::string                TypeString;
        std::vector<std::string>   ArgumentTypeStrings;
        xCodeAttribute             CodeAttribute;
    };

    X_GAME_API uint16_t ExtractConstantValueAttribute(const std::vector<xel::ubyte> & Binary);
    X_GAME_API uint16_t ExtractSourceAttribute(const std::vector<xel::ubyte> & Binary);
    X_GAME_API xClassEx Extend(const xClassInfo& JavaClass);
    X_GAME_API xFieldEx Extend(const xClassInfo& JavaClass, const xFieldInfo & FieldInfo);
    X_GAME_API xMethodEx Extend(const xClassInfo& JavaClass, const xMethodInfo & MethodInfo);

}