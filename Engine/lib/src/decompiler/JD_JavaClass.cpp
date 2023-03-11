#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaPackage.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaType.hpp>
#include <xel/Byte.hpp>

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
            Extend.SuggestedSourceFilename = SourceFile->SourceFile;
        } else {
            Extend.SuggestedSourceFilename = GetOutermostClassCodeName(_SimpleBinaryName) + ".java";
        }

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
                X_DEBUG_PRINTF("FoundInnerClass: %s\n", InnerClassBinaryName.c_str());

                auto InnerClassPtr = JavaSpacePtr->GetClass(InnerClassBinaryName);
                assert(InnerClassPtr);
                Extend.DirectInnerClasses.push_back(InnerClassPtr);
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
        Converted.ClassName = _InnermostName;

        // annotations
        if (!DoConvertAnnotations()) {
            return false;
        }

        RewindConvertedGuard.Dismiss();
        return true;
    }

    bool xJavaClass::DoConvertAnnotations()
    {
        auto & Annotations = Converted.AnnotaionDeclarations;


        (void)Annotations;
        return true;
    }


}
