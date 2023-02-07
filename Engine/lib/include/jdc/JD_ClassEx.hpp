#include "./JD_Class.hpp"
#include <xel/Byte.hpp>

namespace jdc
{

    struct xClassEx
    {
        std::string SourceFile;
        std::string FullClassName;
        std::string FullSuperClassName;
        std::vector<std::string> InterfaceNames;
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

    struct xExceptionTableItem
    {
        uint16_t StartPC;
        uint16_t EndPC;
        uint16_t HandlerPC;
        uint16_t CatchType;
    };

    struct xCode
    {
        uint16_t MaxStack;
        uint16_t MaxLocals;
        std::vector<xel::ubyte>             Binary;
        std::vector<xExceptionTableItem>    ExceptionTable;
        std::vector<xAttributeInfo>         Attributes;
    };

    struct xMethodEx
    {
        std::string              Name;
        xAccessFlag              AccessFlags;
        std::string              TypeString;
        std::vector<xel::ubyte>  CodeBinary;
    };

    X_GAME_API uint16_t ExtractConstantValueAttribute(const std::vector<xel::ubyte> & Binary);
    X_GAME_API uint16_t ExtractSourceAttribute(const std::vector<xel::ubyte> & Binary);
    X_GAME_API xClassEx Extend(const xClass& JavaClass);
    X_GAME_API xFieldEx Extend(const xClass& JavaClass, const xFieldInfo & FieldInfo);
    X_GAME_API xMethodEx Extend(const xClass& JavaClass, const xMethodInfo & MethodInfo);

}
