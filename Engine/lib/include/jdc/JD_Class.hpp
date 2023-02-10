#pragma once
#include "./JD_Base.hpp"
#include "./JD_BaseError.hpp"
#include <xel/Byte.hpp>
#include <string>
#include <vector>

namespace jdc
{

    enum struct eFieldType : uint8_t
    {
        Void,
        Byte,
        Short,
        Integer,
        Long,
        Char,
        Float,
        Double,
        Boolean,
        Class,
        Array,

        // the following are used only when extracting types form descriptor:
        ParamStart,
        ParamEnd,
        Invalid,
    };

    enum struct eConstantTag : uint8_t
    {
        Unspecified        = 0,
        Utf8               = 1,
        Integer            = 3,
        Float              = 4,
        Long               = 5,
        Double             = 6,
        Class              = 7,
        String             = 8,
        FieldRef           = 9,
        MethodRef          = 10,
        InterfaceMethodRef = 11,
        NameAndType        = 12,
        MethodHandle       = 15,
        MethodType         = 16,
        Dynamic            = 17,
        InvokeDynamic      = 18,
        Module             = 19,
        Package            = 20,
    };

    constexpr bool IsLoadableConstantTag(const eConstantTag Tag)
    {
        switch (Tag) {
            case eConstantTag::Integer:
            case eConstantTag::Float:
            case eConstantTag::Long:
            case eConstantTag::Double:
            case eConstantTag::Class:
            case eConstantTag::String:
            case eConstantTag::MethodHandle:
            case eConstantTag::MethodType:
            case eConstantTag::Dynamic:
                return true;
            default:
                break;
        }
        return false;
    }

    using xAccessFlag = uint16_t;
    constexpr const xAccessFlag ACC_PUBLIC       = 0x0001; // field | method
    constexpr const xAccessFlag ACC_PRIVATE      = 0x0002; // field | method
    constexpr const xAccessFlag ACC_PROTECTED    = 0x0004; // field | method
    constexpr const xAccessFlag ACC_STATIC       = 0x0008; // field | method
    constexpr const xAccessFlag ACC_FINAL        = 0x0010; // field | method
    constexpr const xAccessFlag ACC_SYNCHRONIZED = 0x0020; // method
    constexpr const xAccessFlag ACC_SUPER        = 0x0020; // class | interface
    constexpr const xAccessFlag ACC_OPEN         = 0x0020; // module
    constexpr const xAccessFlag ACC_BRIDGE       = 0x0040; // method
    constexpr const xAccessFlag ACC_VOLATILE     = 0x0040; // field
    constexpr const xAccessFlag ACC_VARARGS      = 0x0080; // method
    constexpr const xAccessFlag ACC_TRANSIENT    = 0x0080; // field
    constexpr const xAccessFlag ACC_NATIVE       = 0x0100; // method
    constexpr const xAccessFlag ACC_INTERFACE    = 0x0200; // class
    constexpr const xAccessFlag ACC_ABSTRACT     = 0x0400; // method
    constexpr const xAccessFlag ACC_STRICT       = 0x0800; // method
    constexpr const xAccessFlag ACC_SYNTHETIC    = 0x1000; // class
    constexpr const xAccessFlag ACC_ANNOTATION   = 0x2000; // class
    constexpr const xAccessFlag ACC_ENUM         = 0x4000; // class
    constexpr const xAccessFlag ACC_MODULE       = 0x8000; // class
    constexpr const xAccessFlag ACC_MANDATED     = 0x8000; // module

    struct xConstantClassInfo
    {
        uint16_t PathNameIndex;
    };

    struct xConstantFieldRefInfo
    {
        uint16_t ClassIndex;
        uint16_t NameAndTypeIndex;
    };

    struct xConstantStringInfo
    {
        uint16_t StringIndex;
    };

    struct xConstantMethodRefInfo
    {
        uint16_t ClassIndex;
        uint16_t NameAndTypeIndex;
    };

    struct xConstantInterfaceMethodRefInfo
    {
        uint16_t ClassIndex;
        uint16_t NameAndTypeIndex;
    };

    struct xConstantIntegerInfo
    {
        int32_t Value;
    };

    struct xConstantFloatInfo
    {
        float Value;
    };

    struct xConstantLongInfo
    {
        int64_t Value;
    };

    struct xConstantDoubleInfo
    {
        double Value;
    };

    struct xConstantNameAndTypeInfo
    {
        uint16_t NameIndex;
        uint16_t DescriptorIndex;
    };

    struct xConstantUtf8Info
    {
        std::string * DataPtr;
    };

    struct xConstantMethodHandleInfo
    {
        uint16_t ReferenceKind;
        uint16_t ReferenceIndex;
    };

    struct xConstantMethodTypeInfo
    {
        uint16_t DescriptorIndex;
    };

    struct xConstantDynamicInfo
    {
        uint16_t BootstrapMethodAttributeIndex;
        uint16_t NameAndTypeIndex;
    };

    struct xConstantInvokeDynamicInfo
    {
        uint16_t BootstrapMethodAttributeIndex;
        uint16_t NameAndTypeIndex;
    };

    struct xConstantModuleInfo
    {
        uint16_t NameIndex;
    };

    struct xConstantPackageInfo
    {
        uint16_t NameIndex;
    };

    struct xConstantItemInfo
    {
        eConstantTag         Tag = eConstantTag::Unspecified;
        union {
            xConstantClassInfo                Class;
            xConstantFieldRefInfo             FieldRef;
            xConstantStringInfo               String;
            xConstantMethodRefInfo            MethodRef;
            xConstantInterfaceMethodRefInfo   InterfaceMethodRef;
            xConstantIntegerInfo              Integer;
            xConstantFloatInfo                Float;
            xConstantLongInfo                 Long;
            xConstantDoubleInfo               Double;
            xConstantNameAndTypeInfo          NameAndType;
            xConstantUtf8Info                 Utf8;
            xConstantMethodHandleInfo         MethodHandle;
            xConstantMethodTypeInfo           MethodType;
            xConstantDynamicInfo              Dynamic;
            xConstantInvokeDynamicInfo        InvokeDynamic;
            xConstantModuleInfo               Module;
            xConstantPackageInfo              Package;
        } Info;

        xConstantItemInfo() = default;
        xConstantItemInfo(const xConstantItemInfo &Other);
        xConstantItemInfo(xConstantItemInfo && Other);
        X_GAME_API_MEMBER ~xConstantItemInfo();
        X_GAME_API_MEMBER void SetUtf8(const char * DataPtr, size_t Length);
        X_GAME_API_MEMBER void Clear();
    };

    struct xAttributeInfo
    {
        uint16_t                      NameIndex;
        std::vector<xel::ubyte>       Binary;
    };

    struct xMethodInfo
    {
        uint16_t                      AccessFlags;
        uint16_t                      NameIndex;
        uint16_t                      DescriptorIndex;
        std::vector<xAttributeInfo>   Attributes;
    };

    struct xVariableType
    {
        eFieldType   FieldType;
        std::string  ClassPathName;
    };

    struct xMethodDescriptor
    {
        std::vector<xVariableType>   ParameterTypes;
        xVariableType                ReturnType;
    };

    struct xFieldInfo
    {
        xAccessFlag                   AccessFlags;
        uint16_t                      NameIndex;
        uint16_t                      DescriptorIndex;
        std::vector<xAttributeInfo>   Attributes;
    };

    struct xClass
    {
        uint32_t                           Magic;
        uint16_t                           MinorVersion;
        uint16_t                           MajorVersion;
        std::vector<xConstantItemInfo>     ConstantPool;
        uint16_t                           AccessFlags;
        uint16_t                           ThisClass;
        uint16_t                           SuperClass;
        std::vector<uint16_t>              InterfaceIndices;
        std::vector<xFieldInfo>            Fields;
        std::vector<xMethodInfo>           Methods;
        std::vector<xAttributeInfo>        Attributes;
    };

    struct xExceptionTableItem
    {
        uint16_t StartPC;
        uint16_t EndPC;
        uint16_t HandlerPC;
        uint16_t CatchType;
    };

    struct xCodeAttribute
    {
        bool                                Enabled = false;
        uint16_t                            MaxStack;
        uint16_t                            MaxLocals;
        std::vector<xel::ubyte>             Binary;
        std::vector<xExceptionTableItem>    ExceptionTable;
        std::vector<xAttributeInfo>         Attributes;
    };


    X_GAME_API const char * ClassVersionString(uint16_t MajorVersion);
    X_GAME_API const char * ConstantTagString(const eConstantTag Tag);
    X_GAME_API const char * FieldTypeString(const eFieldType Type);
    X_GAME_API std::string VariableTypeString(const xVariableType & VType);
    X_GAME_API std::string VariableTypeString(const std::string & Utf8);

    X_GAME_API const std::string * GetConstantItemUtf8(const xConstantItemInfo & Item);
    X_GAME_API const std::string * GetConstantItemUtf8(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API const std::string * GetConstantItemString(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API const std::string * GetConstantItemClassPathName(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API const std::string ConstantValueString(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API const std::string ConstantFieldValueString(eFieldType FieldType, const std::vector<xConstantItemInfo> & Items, size_t Index);

    X_GAME_API std::string GetPackageName(const std::string & ClassPathName);
    X_GAME_API std::string GetFullClassName(const std::string & ClassPathName);
    X_GAME_API std::string GetClassName(const std::string & ClassPathName);
    X_GAME_API std::pair<std::string, std::string> GetPackageAndClassName(const std::string & ClassPathName);

    X_GAME_API xVariableType ExtractVariableType(const std::string & Utf8, size_t & Index);
    X_GAME_API xMethodDescriptor ExtractMethodDescriptor(const std::string & Utf8);

    X_GAME_API bool ExtractAttributeInfo(xel::xStreamReader & Reader, ssize_t & RemainSize, xAttributeInfo & AttributeInfo);
    X_GAME_API bool ExtractFieldInfo(xel::xStreamReader & Reader, ssize_t & RemainSize, xFieldInfo & FieldInfo);
    X_GAME_API bool ExtractCodeAttribute(const std::vector<xel::ubyte> & Binary, xCodeAttribute & Output);
    X_GAME_API xJDResult<xClass> LoadClassInfoFromFile(const std::string & Filename);

}
