#pragma once
#include "../base/JD_.hpp"
#include "../base/JD_Bytecode.hpp"
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
        eReferenceKind  ReferenceKind;
        uint16_t        ReferenceIndex;
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

    struct xConstantInfo
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

        xConstantInfo() = default;
        xConstantInfo(const xConstantInfo &Other);
        xConstantInfo(xConstantInfo && Other);
        X_GAME_API_MEMBER ~xConstantInfo();

        X_GAME_API_MEMBER void SetUtf8(const char * DataPtr, size_t Length);
        X_GAME_API_MEMBER void Clear();
    };

}
