#pragma once
#include "./JD_Base.hpp"
#include "./JD_BaseError.hpp"
#include <string>
#include <vector>

namespace jdc
{

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

    enum eFieldType : uint8_t
    {
        Byte    = (uint8_t)'B',
        Char    = (uint8_t)'C',
        Double  = (uint8_t)'D',
        Float   = (uint8_t)'F',
        Integer = (uint8_t)'I',
        Long    = (uint8_t)'J',
        Class   = (uint8_t)'L',
        Short   = (uint8_t)'S',
        Boolean = (uint8_t)'Z',
        Array   = (uint8_t)'[',
    };

    using xAccessFlag = uint16_t;
    constexpr const xAccessFlag ACC_PUBLIC       = 0x0001; // field | method
    constexpr const xAccessFlag ACC_PRIVATE      = 0x0002; // field | method
    constexpr const xAccessFlag ACC_PROTECTED    = 0x0004; // field | method
    constexpr const xAccessFlag ACC_STATIC       = 0x0008; // field | method
    constexpr const xAccessFlag ACC_FINAL        = 0x0010; // field | method
    constexpr const xAccessFlag ACC_SYNCHRONIZED = 0x0020; // method
    constexpr const xAccessFlag ACC_SUPER        = 0x0020; // class | interface
    constexpr const xAccessFlag ACC_OPEN         = 0x0020; // module
    constexpr const xAccessFlag ACC_VOLATILE     = 0x0040; // field | method
    constexpr const xAccessFlag ACC_TRANSIENT    = 0x0080; // field | method
    constexpr const xAccessFlag ACC_NATIVE       = 0x0100; // method
    constexpr const xAccessFlag ACC_INTERFACE    = 0x0200; // class
    constexpr const xAccessFlag ACC_ABSTRACT     = 0x0400; // method
    constexpr const xAccessFlag ACC_STRICT       = 0x0800; // method
    constexpr const xAccessFlag ACC_SYNTHETIC    = 0x1000; // class NOT_IN_SOURCE
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
        uint32_t Value;
    };

    struct xConstantFloatInfo
    {
        float Value;
    };

    struct xConstantLongInfo
    {
        uint64_t Value;
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

    struct xClassMethodInfo
    {

    };

    struct xClassAttributeInfo
    {

    };

    struct xClassFieldInfo
    {
        xAccessFlag                        AccessFlags;
        uint16_t                           NameIndex;
        uint16_t                           DescriptorIndex;
        std::vector<xClassAttributeInfo>   Attributes;
    };

    struct xClass
    {
        uint32_t                           Magic;
        uint16_t                           MinorVersion;
        uint16_t                           MajorVersion;
        std::vector<xConstantItemInfo>     ConstantPoolInfo;
        uint16_t                           AccessFlags;
        uint16_t                           ThisClass;
        uint16_t                           SuperClass;
        std::vector<uint16_t>              InterfaceIndices;
        std::vector<xClassFieldInfo>       Fields;
        std::vector<xClassMethodInfo>      Methods;
        std::vector<xClassAttributeInfo>   Attributes;
    };

    X_GAME_API const char * ConstantTagString(const eConstantTag Tag);

    X_GAME_API const std::string * GetConstantItemUtf8(const xConstantItemInfo & Item);
    X_GAME_API const std::string * GetConstantItemUtf8(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API const std::string * GetConstantItemString(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API const std::string * GetConstantItemClassPathName(const std::vector<xConstantItemInfo> & Items, size_t Index);

    X_GAME_API const char * ClassVersionString(uint16_t MajorVersion);
    X_GAME_API xJDResult<xClass> LoadClassInfoFromFile(const std::string & Filename);

    X_INLINE bool HasClassAccessFlag_Super(xAccessFlag Flag) { return Flag & ACC_SUPER; }
    X_INLINE bool HasClassAccessFlag_Final(xAccessFlag Flag) { return Flag & ACC_FINAL; }
    X_INLINE bool HasClassAccessFlag_Public(xAccessFlag Flag) { return Flag & ACC_PUBLIC; }
    X_INLINE bool HasClassAccessFlag_Interface(xAccessFlag Flag) { return Flag & ACC_INTERFACE; }
    X_INLINE bool HasClassAccessFlag_Abstract(xAccessFlag Flag) { return Flag & ACC_ABSTRACT; }
    X_INLINE bool HasClassAccessFlag_Synthetic(xAccessFlag Flag) { return Flag & ACC_SYNTHETIC; } // not in source code
    X_INLINE bool HasClassAccessFlag_Annotation(xAccessFlag Flag) { return Flag & ACC_ANNOTATION; }
    X_INLINE bool HasClassAccessFlag_Enum(xAccessFlag Flag) { return Flag & ACC_ENUM; }
    X_INLINE bool HasClassAccessFlag_Module(xAccessFlag Flag) { return Flag & ACC_MODULE; }
}
