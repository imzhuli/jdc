#pragma once
#include "../base/JD_Base.hpp"
#include <xel/Byte.hpp>
#include <string>
#include <vector>

namespace jdc
{

    struct xConstantClassInfo
    {
        uint16_t BinaryNameIndex;
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

    struct xFieldInfo
    {
        xAccessFlag                   AccessFlags;
        uint16_t                      NameIndex;
        uint16_t                      DescriptorIndex;
        std::vector<xAttributeInfo>   Attributes;
    };

    struct xClassInfo
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

        X_INLINE bool IsEnum() const { return AccessFlags & ACC_ENUM; }
        X_INLINE bool IsAnnotaion() const { return AccessFlags & ACC_ANNOTATION; }
        X_GAME_API_MEMBER const std::string & GetConstantUtf8(size_t Index) const;
        X_GAME_API_MEMBER const std::string & GetConstantString(size_t Index) const;
        X_GAME_API_MEMBER const std::string & GetConstantClassBinaryName(size_t Index) const;
        X_GAME_API_MEMBER const std::string GetOutermostClassBinaryName() const;
        X_GAME_API_MEMBER const std::string GetConstantValueString(size_t Index) const;
        X_GAME_API_MEMBER const std::vector<std::string> ExtractTypeBinaryNames(const std::string & Descriptor) const;
    };

    struct xExceptionTableItem
    {
        uint16_t StartPC;
        uint16_t EndPC;
        uint16_t HandlerPC;
        uint16_t CatchType;
    };

    struct xInnerClassAttribute
    {
        uint16_t InnerClassInfoIndex;
        uint16_t OuterClassInfoIndex;
        uint16_t InnerNameIndex;
        uint16_t InnerAccessFlags;
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

    X_GAME_API std::string EscapeString(const std::string & S);
    X_GAME_API std::string EscapeStringQuoted(const std::string & S);

    X_GAME_API bool ExtractAttributeInfo(xel::xStreamReader & Reader, ssize_t & RemainSize, xAttributeInfo & AttributeInfo);
    X_GAME_API bool ExtractFieldInfo(xel::xStreamReader & Reader, ssize_t & RemainSize, xFieldInfo & FieldInfo);
    X_GAME_API bool ExtractInnerClassAttribute(const std::vector<xel::ubyte> & Binary, std::vector<xInnerClassAttribute> & Output);
    X_GAME_API bool ExtractCodeAttribute(const std::vector<xel::ubyte> & Binary, xCodeAttribute & Output);

    X_INLINE std::string MakeArgumentName(size_t Index) { return "__arg_" + std::to_string(Index); }
    X_INLINE std::string MakeVariableName(size_t Index) { return "__var_" + std::to_string(Index); }
    X_INLINE bool IsLocalVariableName(const std::string & Name) { return 0 == Name.find("__var_"); }

    X_GAME_API xResult<xClassInfo> LoadClassInfoFromFile(const std::string & Filename);

}
