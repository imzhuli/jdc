#include "./JD_Class.hpp"
#include <xel/Byte.hpp>

namespace jdc
{

    struct xFieldEx
    {
        std::string        Name;
        xAccessFlag        AccessFlags;
        xVariableType      Type;
        std::string        TypeString;
        std::string        InitValueString;
    };

    X_GAME_API uint16_t ExtractConstantValueAttribute(const std::vector<xel::ubyte> & Binary);
    X_GAME_API xFieldEx Extend(const xClass& JavaClass, const xFieldInfo & FieldInfo);

}
