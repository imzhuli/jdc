#pragma once
#include "../base/_.hpp"
#include "./JD_ElementValue.hpp"
#include <xel/Byte.hpp>
#include <vector>
#include <string>

namespace jdc
{
    class xClassInfo;

    using xAttributeBinary = std::vector<xel::ubyte>;

    struct xAttributeInfo
    {
        uint16_t                      NameIndex;
        std::vector<xel::ubyte>       Binary;
    };

    struct xAttributeBootstrapMethods
    {
        struct xBootstrapMethod
        {
            uint16_t                 ReferenceIndex;
            std::vector<uint16_t>    ArgumentIndices;
        };

        std::vector<xBootstrapMethod> BootstrapMethods;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeCode
    {
        struct xExceptionTable
        {
            uint16_t StartPC;
            uint16_t EndPC;
            uint16_t HandlePC;
            uint16_t CatchType;
        };

        uint16_t MaxStack;
        uint16_t MaxLocals;
        std::vector<xel::ubyte>      CodeBinary;
        std::vector<xExceptionTable> ExceptionTables;
        std::vector<xAttributeInfo>  SubAttributes;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

    struct xAttributeDeprecated
    {
        bool Deprecated = false;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

    struct xAttributeExceptions
    {
        std::vector<uint16_t> ExceptionIndexTable;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

    struct xAttributeInnerClasses
    {
        uint16_t InnerClassInfoIndex;
        uint16_t OuterClassInfoIndex;
        uint16_t InnerNameIndex;
        uint16_t InnerAccessFlags;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

    struct xAttributeLineNumberTable
    {
        struct xLineNumber
        {
            uint16_t StartPC;
            uint16_t LineNumber;
        };

        std::vector<xLineNumber> LineNumberTable;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

    struct xAttributeLocalVariableTable
    {
        struct xLocalVariable {
            uint16_t     StartPC;
            uint16_t     Length;
            std::string  Name;       // from: uint16_t NameIndex;
            std::string  Descriptor; // uint16_t DescriptorIndex;
            uint16_t     Index;
        };

        std::vector<xLocalVariable> LocalVariableTable;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeLocalVariableTypeTable
    {
        struct xLocalVariableType {
            uint16_t     StartPC;
            uint16_t     Length;
            std::string  Name;
            std::string  Signature;
            uint16_t     Index;
        };

        std::vector<xLocalVariableType> LocalVariableTypeTable;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeMethodParameters
    {
        struct xParameter {
            std::string Name;
            uint16_t    AccessFlags;
        };

        std::vector<xParameter> Parameters;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeModule
    {
        struct xRequires {
            uint16_t Index;
            uint16_t Flags;
            uint16_t VersionIndex;
        };

        struct xExports {
            uint16_t Index;
            uint16_t Flags;
            std::vector<uint16_t> ExportsToIndices;
        };

        struct xOpens {
            uint16_t Index;
            uint16_t Flags;
            std::vector<uint16_t> OpensToIndices;
        };

        struct xUses {
            uint16_t  Index;
        };

        struct xProvides {
            uint16_t              Index;
            std::vector<uint16_t> ProvidesWithIndex;
        };

        std::string              Name;
        uint16_t                 Flags;
        std::string              VersionIndex;
        std::vector<xRequires>   RequiresList;
        std::vector<xExports>    ExportsList;
        std::vector<xOpens>      OpensList;
        std::vector<xUses>       UsesList;
        std::vector<xProvides>   ProvidesList;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xModuleMainClass
    {
        std::string ClassName;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xModulePackages
    {
        std::vector<std::string> PackageNames;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeParameterAnnotations
    {
        struct xAnnotation
        {
            std::string                       Descriptor;
            std::vector<xElementValuePair>    ElementValuePairs;
        };
        std::vector<xAnnotation> Annotations;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeSignature
    {
        std::string Signature;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeSourceFile
    {
        std::string SourceFile;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeSynthetic
    {
        bool Synthetic = false;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

}
