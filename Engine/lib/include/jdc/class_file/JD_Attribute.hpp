#pragma once
#include "../base/_.hpp"
#include <xel/Byte.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace jdc
{
    class xClassInfo;

    struct xAnnotation;
    struct xElementValue;
    struct xElementValuePair;

    struct xElementValue
    {
        eElementValueTag Tag;
        union {
            uint16_t ConstantValueIndex;
            uint16_t ClassIndex;
            struct {
                uint16_t TypeNameIndex;
                uint16_t NameIndex;
            } EnumConstantValue;
        };
        std::unique_ptr<xAnnotation>                   AnnotationUPtr;
        std::vector<std::unique_ptr<xElementValue>>    ArrayValues;
    };

    struct xElementValuePair
    {
        uint16_t                         ElementNameIndex;
        std::unique_ptr<xElementValue>   ElementValueUPtr;
    };

    struct xAnnotation
    {
        uint16_t                         TypeNameIndex;
        std::vector<xElementValuePair>   ElementValuePairs;
    };

    using xAttributeBinary = std::vector<xel::ubyte>;

    struct xAttributeInfo
    {
        uint16_t                      NameIndex;
        std::vector<xel::ubyte>       Binary;
    };

    struct xAttributeBase : private xel::xAbstract {};

    struct xAttributeBootstrapMethods : public xAttributeBase
    {
        struct xBootstrapMethod
        {
            uint16_t                 ReferenceIndex;
            std::vector<uint16_t>    ArgumentIndices;
        };

        std::vector<xBootstrapMethod> BootstrapMethods;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeConstantValue : public xAttributeBase
    {
        uint16_t ValueIndex;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeCode : public xAttributeBase
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

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeDeprecated : public xAttributeBase
    {
        bool Deprecated = false;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeExceptions : public xAttributeBase
    {
        std::vector<uint16_t> ExceptionIndexTable;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeInnerClasses : public xAttributeBase
    {
        uint16_t InnerClassInfoIndex;
        uint16_t OuterClassInfoIndex;
        uint16_t InnerNameIndex;
        uint16_t InnerAccessFlags;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeLineNumberTable : public xAttributeBase
    {
        struct xLineNumber
        {
            uint16_t StartPC;
            uint16_t LineNumber;
        };

        std::vector<xLineNumber> LineNumberTable;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeLocalVariableTable : public xAttributeBase
    {
        struct xLocalVariable {
            uint16_t     StartPC;
            uint16_t     Length;
            std::string  Name;       // from: uint16_t NameIndex;
            std::string  Descriptor; // uint16_t DescriptorIndex;
            uint16_t     Index;
        };

        std::vector<xLocalVariable> LocalVariableTable;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeLocalVariableTypeTable : public xAttributeBase
    {
        struct xLocalVariableType {
            uint16_t     StartPC;
            uint16_t     Length;
            std::string  Name;
            std::string  Signature;
            uint16_t     Index;
        };

        std::vector<xLocalVariableType> LocalVariableTypeTable;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeMethodParameters : public xAttributeBase
    {
        struct xParameter {
            std::string Name;
            uint16_t    AccessFlags;
        };

        std::vector<xParameter> Parameters;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeModule : public xAttributeBase
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

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeModulePackages : public xAttributeBase
    {
        std::vector<std::string> PackageNames;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeModuleMainClass : public xAttributeBase
    {
        std::string ClassName;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeRuntimeAnnotations : public xAttributeBase
    {
        std::vector<std::unique_ptr<xAnnotation>> Annotations;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeRuntimeParameterAnnotations : public xAttributeBase
    {
        std::vector<
            std::vector<std::unique_ptr<xAnnotation>>
        > ParameterAnnotations;
        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeSignature : public xAttributeBase
    {
        std::string Signature;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeSourceFile : public xAttributeBase
    {
        std::string SourceFile;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeSynthetic : public xAttributeBase
    {
        bool Synthetic = false;

        X_PRIVATE_MEMBER bool Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr);
    };

    struct xAttributeNames{
        static constexpr const char * AnnotationDefault                      = "AnnotationDefault";
        static constexpr const char * BootstrapMethods                       = "BootstrapMethods";
        static constexpr const char * Code                                   = "Code";
        static constexpr const char * ConstantValue                          = "ConstantValue";
        static constexpr const char * Deprecated                             = "Deprecated";
        static constexpr const char * EnclosingMethod                        = "EnclosingMethod";
        static constexpr const char * Exceptions                             = "Exceptions";
        static constexpr const char * InnerClasses                           = "InnerClasses";
        static constexpr const char * LocalVariableTable                     = "LocalVariableTable";
        static constexpr const char * LocalVariableTypeTable                 = "LocalVariableTypeTable";
        static constexpr const char * LineNumberTable                        = "LineNumberTable";
        static constexpr const char * MethodParameters                       = "MethodParameters";
        static constexpr const char * Module                                 = "Module";
        static constexpr const char * ModulePackages                         = "ModulePackages";
        static constexpr const char * ModuleMainClass                        = "ModuleMainClass";
        static constexpr const char * RuntimeInvisibleAnnotations            = "RuntimeInvisibleAnnotations";
        static constexpr const char * RuntimeVisibleAnnotations              = "RuntimeVisibleAnnotations";
        static constexpr const char * RuntimeInvisibleParameterAnnotations   = "RuntimeInvisibleParameterAnnotations";
        static constexpr const char * RuntimeVisibleParameterAnnotations     = "RuntimeVisibleParameterAnnotations";
        static constexpr const char * Signature                              = "Signature";
        static constexpr const char * SourceFile                             = "SourceFile";
        static constexpr const char * Synthetic                              = "Synthetic";
    };

    X_PRIVATE std::unique_ptr<xElementValue> LoadElementValue(xel::xStreamReader & Reader);
    X_PRIVATE xElementValuePair              LoadElementValuePair(xel::xStreamReader & Reader);
    X_PRIVATE std::unique_ptr<xAnnotation>   LoadAnnotation(xel::xStreamReader & Reader);

    using xAttributeMap = std::unordered_map<std::string, std::unique_ptr<xAttributeBase>>;
    X_PRIVATE xAttributeMap LoadAttributeInfo(const std::vector<xAttributeInfo> & AttributeInfoList, const xClassInfo * ClassInfoPtr);
    X_INLINE const xAttributeBase * GetAttribute(const xAttributeMap & Map, const char * Name) {
        auto Iter = Map.find(Name);
        if (Iter == Map.end()) {
            return nullptr;
        }
        return Iter->second.get();
    }

}
