#pragma once
#include "../base/_.hpp"
#include "../base/JD_Instructions.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "./JD_JavaPackage.hpp"
#include "./JD_JavaField.hpp"
#include "./JD_JavaMethod.hpp"
#include "./JD_JavaDeclaration.hpp"
#include <string>
#include <cctype>
#include <vector>
#include <ostream>

namespace jdc
{

    class xJavaPackage;
    class xJavaClass;
    class xJavaSpace;

    class xJavaClass final
    : public iJavaType
    {
        friend class xJavaSpace;

    public:
        const xJavaSpace *                    JavaSpacePtr = nullptr;
        const xJavaPackage *                  PackagePtr = nullptr;
        xClassInfo                            ClassInfo;

        struct {
            xAttributeMap                     AttributeMap;
            std::string                       SuggestedSourceFilename;
            std::vector<xJavaField>           Fields;
            std::vector<xJavaMethod>          Methods;
            std::vector<const xJavaClass*>    DirectInnerClasses;
        } Extend;

        struct {
            std::string                       SourceFilename;
            std::string                       PackageName;

            xAccessFlag                       ClassAccessFlags;
            std::string                       ClassName;
            std::string                       SuperClassName;
            std::vector<std::string>          InterfaceNames;

            xAnnotationDeclarations           AnnotaionDeclarations;
        } Converted;

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
        X_INLINE bool IsSynthetic() const { return (ClassInfo.AccessFlags & ACC_SYNTHETIC) || isdigit(_InnermostName[0]); }
        X_INLINE bool IsInnerClass() const { return _SimpleCodeName.length() != _InnermostName.length(); }
        X_INLINE bool IsMainClass() const { return !IsInnerClass() && (_SourceFilename.empty() ? true : (_SourceFilename == _SimpleCodeName)); }

        X_PRIVATE_MEMBER std::string GetUnfixedOutermostClassBinaryName() const;
        X_PRIVATE_MEMBER xJavaField ExtractField(const xFieldInfo & FieldInfo);
        X_PRIVATE_MEMBER xJavaMethod ExtractMethod(const xMethodInfo & MethodInfo);

        X_PRIVATE_MEMBER void DoExtend();

        X_PRIVATE_MEMBER bool DoConvert();
        X_PRIVATE_MEMBER bool DoConvertFields();
        X_PRIVATE_MEMBER bool DoConvertAnnotations();
        X_PRIVATE_MEMBER bool DoConvertMethods();

        X_PRIVATE_MEMBER bool DumpSourceToFile(const std::string & RootDir) const;
        X_PRIVATE_MEMBER bool DumpSource(std::ofstream & OS, size_t Level) const;
        X_PRIVATE_MEMBER bool DumpPackageFragment(std::ostream & OS) const;
        X_PRIVATE_MEMBER bool DumpSpacerLineFragment(std::ostream & OS) const;
        X_PRIVATE_MEMBER bool ClassDeclarationBeginFragment(std::ostream & OS, size_t Level) const;
        X_PRIVATE_MEMBER bool ClassDeclarationEndFragment(std::ostream & OS, size_t Level) const;

        X_PRIVATE_STATIC_CONSTEXPR const char * IndentString = "    ";
        X_INLINE void DumpInsertLineIndent(std::ostream & OS, size_t Level) const { for (size_t i = 0 ; i < Level; ++i) { OS << IndentString; } }
    };

}