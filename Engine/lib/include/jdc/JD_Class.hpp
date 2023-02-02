#pragma once
#include "./JD_Base.hpp"
#include "./JD_BaseError.hpp"
#include <string>
#include <vector>

namespace xjd
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

    struct xConstantClassInfo
    {
        uint16_t NameIndex;
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

    struct xConstnatNameAndTypeInfo
    {
        uint16_t NameIndex;
        uint16_t DescriptorIndex;
    };

    struct xConstantUtf8Info
    {
        std::string * DataPtr;
    };

    struct xConstMethodHandleInfo
    {
        uint16_t ReferenceKind;
        uint16_t ReferenceIndex;
    };

    struct xConstMethodTypeInfo
    {
        uint16_t DescriptorIndex;
    };

    struct xConstantItemInfo
    {
        eConstantTag         Tag = eConstantTag::Unspecified;
        union {
            xConstantClassInfo                ClassInfo;
            xConstantFieldRefInfo             FieldRefInfo;
            xConstantStringInfo               StringInfo;
            xConstantMethodRefInfo            MethodRefInfo;
            xConstantInterfaceMethodRefInfo   InterfaceMethodRefInfo;
            xConstantIntegerInfo              IntegerInfo;
            xConstantFloatInfo                FloatInfo;
            xConstantLongInfo                 LongInfo;
            xConstantDoubleInfo               DoubleInfo;
            xConstnatNameAndTypeInfo          NameAndTypeInfo;
            xConstantUtf8Info                 Utf8Info;
            xConstMethodHandleInfo            MethodHandleInfo;
            xConstMethodTypeInfo              MethodTypeInfo;
        };

        xConstantItemInfo() = default;
        xConstantItemInfo(const xConstantItemInfo &Other);
        X_GAME_API_MEMBER ~xConstantItemInfo();
        X_GAME_API_MEMBER void SetUtf8(const char * DataPtr, size_t Length);
        X_GAME_API_MEMBER void Clear();
    };

    struct xClassInterface
    {

    };

    struct xClassFieldInfo
    {

    };

    struct xClassMethodInfo
    {

    };

    struct xClassAttributeInfo
    {

    };

    struct xClass
    {
        std::string                        Name;
        uint32_t                           Magic;
        uint16_t                           MinorVersion;
        uint16_t                           MajorVersion;
        std::vector<xConstantItemInfo>     ConstantPoolInfo;
        uint16_t                           AccessFlags;
        uint16_t                           ThisClass;
        uint16_t                           SuperClass;
        std::vector<xClassInterface>       Interfaces;
        std::vector<xClassFieldInfo>       Fields;
        std::vector<xClassMethodInfo>      Methods;
        std::vector<xClassAttributeInfo>   Attributes;
    };

    X_GAME_API const char * ConstantTypeString(const eConstantTag Tag);
    X_GAME_API const char * ClassVersionString(uint16_t MajorVersion);
    X_GAME_API xJDResult<xClass> LoadClassInfoFromFile(const std::string & Filename);
    X_GAME_API std::string DumpStringFromClass(const xClass & JavaClass);
}
