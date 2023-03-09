#pragma once
#include "../base/_.hpp"
#include "../base/JD_Instructions.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "./JD_JavaPackage.hpp"
#include "./JD_JavaMethod.hpp"
#include <string>
#include <cctype>
#include <vector>

namespace jdc
{

    class xJavaPackage;
    class xJavaClass;
    class xJavaSpace;

    class xJavaClass
    : public iJavaType
    {
        friend class xJavaSpace;

    public:
        const xJavaSpace *   JavaSpacePtr = nullptr;
        const xJavaPackage * PackagePtr = nullptr;
        xClassInfo  ClassInfo;

        struct {
            xAttributeDeprecated          AttributeDeprecated;
            xAttributeSynthetic           AttributeSynthetic;
            xAttributeInnerClasses        AttributeInnerClasses;
            xAttributeSourceFile          AttributeSourceFile;
            xAttributeBootstrapMethods    AttributeBootstrapMethods;
            std::vector<xJavaMethod>      Methods;
        } Extend;

        X_INLINE const std::string & GetUnfixedSuperClassBinaryName() const { return ClassInfo.GetConstantClassBinaryName(ClassInfo.SuperClass); }
        X_INLINE std::vector<std::string> GetUnfixedInterfaceBinaryNames() const {
            auto NameCollection = std::vector<std::string>();
            for (auto & Index : ClassInfo.InterfaceIndices) {
                NameCollection.push_back(ClassInfo.GetConstantClassBinaryName(Index));
            }
            return NameCollection;
        }

        X_INLINE bool IsEnum() const { return ClassInfo.AccessFlags & ACC_ENUM; }
        X_INLINE bool IsAnnotation() const { return ClassInfo.AccessFlags & ACC_ANNOTATION; }
        X_INLINE bool IsInterface() const { return ClassInfo.AccessFlags & ACC_INTERFACE; }
        X_INLINE bool IsModule() const { return ClassInfo.AccessFlags & ACC_MODULE; }
        X_INLINE bool IsStatic() const { return ClassInfo.AccessFlags & ACC_STATIC; }
        X_INLINE bool IsAbstract() const { return ClassInfo.AccessFlags & ACC_ABSTRACT; }
        X_INLINE bool IsFinal() const { return ClassInfo.AccessFlags & ACC_FINAL; }
        X_INLINE bool IsSynthetic() const { return (ClassInfo.AccessFlags & ACC_SYNTHETIC) || isdigit(_InnermostCodeName[0]); }
        X_INLINE bool IsInnerClass() const { return _SimpleCodeName.length() != _InnermostCodeName.length(); }
        X_INLINE bool IsMainClass() const { return !IsInnerClass() && (_SourceFilename.empty() ? true : (_SourceFilename == _SimpleCodeName)); }

        X_PRIVATE_MEMBER std::string GetUnfixedOutermostClassBinaryName() const;
        X_PRIVATE_MEMBER xJavaMethod ExtractMethod(size_t Index);
        X_PRIVATE_MEMBER void DoExtend();

    };


}