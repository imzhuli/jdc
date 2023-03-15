#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaPackage.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaType.hpp>
#include <jdc/syntax/JD_JavaObjectTypes.hpp>
#include <xel/Byte.hpp>
#include <xel/String.hpp>
#include <filesystem>
#include <fstream>

using namespace xel;

namespace jdc
{

    std::string xJavaClass::GetUnfixedOutermostClassBinaryName() const
    {
        return ClassInfo.GetOutermostClassBinaryName();
    }

    xJavaField xJavaClass::ExtractField(const xFieldInfo & FieldInfo)
    {
        auto Field = xJavaField();
        Field.ClassInfoPtr = &ClassInfo;
        Field.FieldInfoPtr = &FieldInfo;

        return Field;
    }

    xJavaMethod xJavaClass::ExtractMethod(const xMethodInfo & MethodInfo)
    {
        auto Method = xJavaMethod();
        auto MethodName = ClassInfo.GetConstantUtf8(MethodInfo.NameIndex);
        Method.ClassInfoPtr = &ClassInfo;
        Method.MethodInfoPtr = &MethodInfo;
        Method.OriginalName = MethodName;

        // build method identifier:
        if (MethodName == "<clinit>") {
            Method.Identifier = "static";
        }
        else if (MethodName == "<init>") {
            Method.Identifier = _InnermostName;
        }
        else {
            Method.Identifier = MethodName;
        }

        Method.DoExtend();
        return Method;
    }

    void xJavaClass::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaClass::DoExtend %s --> %s --> %s\n", _FixedBinaryName.c_str(), _SimpleBinaryName.c_str(), _InnermostName.c_str());

        Extend.AttributeMap = LoadAttributeInfo(ClassInfo.Attributes, &ClassInfo);

        auto SourceFile = (const xAttributeSourceFile*)GetAttribute(Extend.AttributeMap, xAttributeNames::SourceFile);
        if (SourceFile) {
            X_DEBUG_PRINTF("Found SourceFile attribute, but ignored: %s\n", SourceFile->Filename.c_str());
            // Extend.SuggestedSourceFilename = SourceFile->Filename;
        }
        Extend.SuggestedSourceFilename = GetOutermostClassCodeName(_SimpleBinaryName) + ".java";

        for (auto & Field : ClassInfo.Fields) {
            Extend.Fields.push_back(ExtractField(Field));
        }

        for (auto & Method : ClassInfo.Methods) {
            Extend.Methods.push_back(ExtractMethod(Method));
        }

        auto AttributeInnerClassesPtr = (xAttributeInnerClasses*)GetAttribute(Extend.AttributeMap, xAttributeNames::InnerClasses);
        if (AttributeInnerClassesPtr) {
            for (auto & InnerClass : AttributeInnerClassesPtr->InnerClasses) {
                auto & OuterClassBinaryName = ClassInfo.GetConstantClassBinaryName(InnerClass.OuterClassInfoIndex);
                if (OuterClassBinaryName != _UnfixedBinaryName) {
                    continue;
                }
                auto & InnerClassBinaryName = ClassInfo.GetConstantClassBinaryName(InnerClass.InnerClassInfoIndex);
                X_DEBUG_PRINTF("FoundInnerClass: %s, AccessFlags=0x%04x\n", InnerClassBinaryName.c_str(), InnerClass.InnerAccessFlags);

                auto InnerClassPtr = JavaSpacePtr->GetClass(InnerClassBinaryName);
                assert(InnerClassPtr);
                Extend.DirectInnerClasses.push_back(InnerClassPtr);
                // fix inner class access_flags:
                InnerClassPtr->ClassInfo.AccessFlags |= InnerClass.InnerAccessFlags;
            }
        }
    }

    bool xJavaClass::DoConvert()
    {
        auto RewindConvertedGuard = xel::xScopeGuard([this]{ xel::Renew(Converted); });

        // class filename:
        Converted.SourceFilename = Extend.SuggestedSourceFilename;

        // package
        Converted.PackageName = PackagePtr->FixedCodeName;

        // class name
        Converted.ClassAccessFlags = ClassInfo.AccessFlags;
        Converted.ClassName = _InnermostName;

        // super class
        auto & UnfixedSuperClassName = GetUnfixedSuperClassBinaryName();
        if (!xJavaObjectType::IsDefaultClassBase(UnfixedSuperClassName)) {
            Converted.SuperClassName = JavaSpacePtr->GetFixedClassCodeName(UnfixedSuperClassName);
        }

        // interfaces
        for (auto & InterfaceIndex : ClassInfo.InterfaceIndices) {
            Converted.InterfaceNames.push_back(JavaSpacePtr->GetFixedClassCodeName(ClassInfo.GetConstantClassBinaryName(InterfaceIndex)));
        }

        // annotations
        if (!DoConvertAnnotations()) {
            return false;
        }

        if (!DoConvertFields()) {
            return false;
        }

        if (!DoConvertMethods()) {
            return false;
        }

        RewindConvertedGuard.Dismiss();
        return true;
    }

    std::string xJavaClass::ConvertElementValueToString(const xElementValue & ElementValue)
    {
        switch (ElementValue.Tag) {
            case eElementValueTag::Byte: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Integer;
                return "(byte)" + ConstValue.Value ? "true" : "false";
            }
            case eElementValueTag::Short: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Integer;
                return "(short)" + std::to_string(ConstValue.Value);
            }
            case eElementValueTag::Int: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Integer;
                return "(int)" + std::to_string(ConstValue.Value);
            }
            case eElementValueTag::Long: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Long;
                return "(long)" + std::to_string(ConstValue.Value);
            }
            case eElementValueTag::Char: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Integer;
                return "(char)" + std::to_string(ConstValue.Value);
            }
            case eElementValueTag::Float: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Float;
                return "(float)" + std::to_string(ConstValue.Value);
            }
            case eElementValueTag::Double: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Double;
                return "(double)" + std::to_string(ConstValue.Value);
            }
            case eElementValueTag::Boolean: {
                auto & ConstValue = ClassInfo.GetConstantInfo(ElementValue.ConstantValueIndex).Details.Double;
                return ConstValue.Value ? "true" : "false";
                break;
            }
            case eElementValueTag::String: {
                auto & ConstValue = ClassInfo.GetConstantUtf8(ElementValue.ConstantValueIndex);
                return '"' + ConstValue + '"';
            }

            // special
            case eElementValueTag::Enum: {
                X_DEBUG_PRINTF("Converting Enum element: \n");
                break;
            }
            case eElementValueTag::Array: {
                break;
            }
            default: {
                X_DEBUG_PRINTF("Non-applicable tag of element: %u:%c\n", (unsigned int)ElementValue.Tag, (char)ElementValue.Tag);
                Fatal("Non-applicable tag of element");
                break;
            }
        }
        return {};
    }

    bool xJavaClass::DoConvertAnnotations()
    {
        auto VisibleAnnotationAttributes = (xAttributeRuntimeAnnotations*)Extend.AttributeMap[xAttributeNames::RuntimeVisibleAnnotations].get();
        auto InvisibleAnnotationAttributes = (xAttributeRuntimeAnnotations*)Extend.AttributeMap[xAttributeNames::RuntimeInvisibleAnnotations].get();

        if (VisibleAnnotationAttributes) {
            for (auto & AA : VisibleAnnotationAttributes->Annotations) {
                auto UnfixedAnnotationBinaryName = ConvertTypeDescriptorToBinaryName(ClassInfo.GetConstantUtf8(AA->TypeNameIndex));
                auto FixedAnnotationCodeName = JavaSpacePtr->GetFixedClassCodeName(UnfixedAnnotationBinaryName);

                auto AD = xAnnotationDeclaration();
                AD.TypeName = FixedAnnotationCodeName;
                for (auto & EVPair : AA->ElementValuePairs) {
                    auto EVStringPair = xElementValueStringPair();
                    EVStringPair.ElementName = ClassInfo.GetConstantUtf8(EVPair.ElementNameIndex);
                    EVStringPair.ElementValueString = ConvertElementValueToString(*EVPair.ElementValueUPtr);
                    AD.ElementValueStringPairs.push_back(EVStringPair);
                }
                Converted.AnnotaionDeclarations.push_back(AD);
            }
        }

        if (InvisibleAnnotationAttributes) {
            for (auto & AA : InvisibleAnnotationAttributes->Annotations) {
                auto UnfixedAnnotationBinaryName = ConvertTypeDescriptorToBinaryName(ClassInfo.GetConstantUtf8(AA->TypeNameIndex));
                auto FixedAnnotationCodeName = JavaSpacePtr->GetFixedClassCodeName(UnfixedAnnotationBinaryName);

                auto AD = xAnnotationDeclaration();
                AD.TypeName = FixedAnnotationCodeName;
                Converted.AnnotaionDeclarations.push_back(AD);
            }
        }

        return true;
    }

    bool xJavaClass::DoConvertFields()
    {
        return true;
    }

    bool xJavaClass::DoConvertMethods()
    {
        return true;
    }

    bool xJavaClass::DumpSourceToFile(const std::string & RootDir) const
    {
        if (IsInnerClass()) {
            return true;
        }

        auto RootPath = std::filesystem::path(RootDir);
        auto PackagePath = PackagePtr->FixedPathName;
        auto Filename = Extend.SuggestedSourceFilename;
        auto FullSourcePath = RootPath / PackagePath / Filename;

        std::ofstream OS(FullSourcePath);
        if (!OS) {
            X_DEBUG_PRINTF("Failed to open class source file: %s\n", FullSourcePath.string().c_str());
            return false;
        }
        return DumpSource(OS, 0);
    }

    bool xJavaClass::DumpSource(std::ofstream & OS, size_t Level) const
    {
        if (!IsInnerClass()) {
            if (!DumpPackageFragment(OS)) {
                return false;
            }
            DumpSpacerLineFragment(OS);
        }

        if (!IsSynthetic()) {
            ClassDeclarationBeginFragment(OS, Level);

            for (auto & InnerClassPtr : Extend.DirectInnerClasses) {
                InnerClassPtr->DumpSource(OS, Level + 1);
            }

            ClassDeclarationEndFragment(OS, Level);
        }
        return true;
    }

    bool xJavaClass::DumpPackageFragment(std::ostream & OS) const
    {
        OS << "package " << Converted.PackageName << ';' << std::endl;
        return true;
    }

    bool xJavaClass::DumpSpacerLineFragment(std::ostream & OS) const
    {
        OS << std::endl;
        return true;
    }

    bool xJavaClass::ClassDeclarationBeginFragment(std::ostream & OS, size_t Level) const
    {
        // TODO: Annotation:
        for (auto & AD : Converted.AnnotaionDeclarations)
        {
            DumpInsertLineIndent(OS, Level);
            // Annotation:
            OS << '@' << AD.TypeName;

            if (AD.ElementValueStringPairs.size()) {
                std::vector<std::string> Params;
                for (auto & EVP : AD.ElementValueStringPairs) {
                    Params.push_back(EVP.ElementName + '=' + EVP.ElementValueString);
                }
                OS << "(" << JoinStr(Params, ',') << ")";
            }
            OS << std::endl;
            // values
        }

        // class
        if (IsEnum()) {
            std::vector<std::string> Qualifiers;
            if (Converted.ClassAccessFlags & ACC_PUBLIC) {
                Qualifiers.push_back("public");
            }
            if (Converted.ClassAccessFlags & ACC_PRIVATE) {
                Qualifiers.push_back("private");
            }
            if (Converted.ClassAccessFlags & ACC_PROTECTED) {
                Qualifiers.push_back("protected");
            }
            if (Converted.ClassAccessFlags & ACC_STATIC) {
                Qualifiers.push_back("static");
            }
            if (Converted.ClassAccessFlags & ACC_FINAL) {
                Qualifiers.push_back("final");
            }
            if (Converted.ClassAccessFlags & ACC_ABSTRACT) {
                Qualifiers.push_back("abstract");
            }

            DumpInsertLineIndent(OS, Level);
            auto QualifierString = JoinStr(Qualifiers, ' ');
            if (!QualifierString.empty()) {
                OS << QualifierString << ' ';
            }
            OS << "enum " << Converted.ClassName << std::endl;

        } else if (IsInterface()) {
            std::vector<std::string> Qualifiers;
            if (Converted.ClassAccessFlags & ACC_PUBLIC) {
                Qualifiers.push_back("public");
            }
            if (Converted.ClassAccessFlags & ACC_PRIVATE) {
                Qualifiers.push_back("private");
            }
            if (Converted.ClassAccessFlags & ACC_PROTECTED) {
                Qualifiers.push_back("protected");
            }
            if (Converted.ClassAccessFlags & ACC_STATIC) {
                Qualifiers.push_back("static");
            }

            DumpInsertLineIndent(OS, Level);
            auto QualifierString = JoinStr(Qualifiers, ' ');
            if (!QualifierString.empty()) {
                OS << QualifierString << ' ';
            }
            if (IsAnnotation()) {
                OS << "@interface " << Converted.ClassName << std::endl;
            } else {
                OS << "interface " << Converted.ClassName << std::endl;
            }
            if (Converted.SuperClassName.size()) {
                DumpInsertLineIndent(OS, Level + 1);
                OS << "extends " << Converted.SuperClassName << std::endl;
            }

        } else {
            std::vector<std::string> Qualifiers;
            if (Converted.ClassAccessFlags & ACC_PUBLIC) {
                Qualifiers.push_back("public");
            }
            if (Converted.ClassAccessFlags & ACC_PRIVATE) {
                Qualifiers.push_back("private");
            }
            if (Converted.ClassAccessFlags & ACC_PROTECTED) {
                Qualifiers.push_back("protected");
            }
            if (Converted.ClassAccessFlags & ACC_STATIC) {
                Qualifiers.push_back("static");
            }
            if (Converted.ClassAccessFlags & ACC_FINAL) {
                Qualifiers.push_back("final");
            }
            if (Converted.ClassAccessFlags & ACC_ABSTRACT) {
                Qualifiers.push_back("abstract");
            }

            DumpInsertLineIndent(OS, Level);
            auto QualifierString = JoinStr(Qualifiers, ' ');
            if (!QualifierString.empty()) {
                OS << QualifierString << ' ';
            }
            OS << "class " << Converted.ClassName << std::endl;

            if (Converted.SuperClassName.size()) {
                DumpInsertLineIndent(OS, Level + 1);
                OS << "extends " << Converted.SuperClassName << std::endl;
            }

            if (Converted.InterfaceNames.size()) {
                auto ImplementsString = JoinStr(Converted.InterfaceNames, ", ");
                DumpInsertLineIndent(OS, Level + 1);
                OS << "implements " << ImplementsString << std::endl;
            }
        }

        DumpInsertLineIndent(OS, Level);
        OS << '{' << std::endl;
        return true;
    }

    bool xJavaClass::ClassDeclarationEndFragment(std::ostream & OS, size_t Level) const
    {
        DumpInsertLineIndent(OS, Level);
        OS << '}' << std::endl;
        return true;
    }

}
