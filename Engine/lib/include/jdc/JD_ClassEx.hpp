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

    struct xInstruction
    {


    };

    struct xMethodEx
    {
        std::string              Name;
        xAccessFlag              AccessFlags;
        std::string              TypeString;
        xCodeAttribute           CodeAttribute;
    };

    X_GAME_API uint16_t ExtractConstantValueAttribute(const std::vector<xel::ubyte> & Binary);
    X_GAME_API uint16_t ExtractSourceAttribute(const std::vector<xel::ubyte> & Binary);
    X_GAME_API xClassEx Extend(const xClass& JavaClass);
    X_GAME_API xFieldEx Extend(const xClass& JavaClass, const xFieldInfo & FieldInfo);
    X_GAME_API xMethodEx Extend(const xClass& JavaClass, const xMethodInfo & MethodInfo);

}
