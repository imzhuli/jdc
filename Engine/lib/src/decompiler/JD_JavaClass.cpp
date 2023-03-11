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
            Method.Identifier = _InnermostCodeName;
        }
        else {
            Method.Identifier = MethodName;
        }

        Method.DoExtend();
        return Method;
    }

    void xJavaClass::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaClass::DoExtend %s --> %s --> %s\n", _FixedBinaryName.c_str(), _SimpleBinaryName.c_str(), _InnermostCodeName.c_str());

        Extend.AttributeMap = LoadAttributeInfo(ClassInfo.Attributes, &ClassInfo);

        auto SourceFile = (const xAttributeSourceFile*)GetAttribute(Extend.AttributeMap, xAttributeNames::SourceFile);
        if (SourceFile) {
            Extend.SuggestedSourceFilename = SourceFile->SourceFile;
        } else {
            Extend.SuggestedSourceFilename = GetOutermostClassCodeName(_SimpleBinaryName) + ".java";
        }

        for (auto & Method : ClassInfo.Methods) {
            Extend.Methods.push_back(ExtractMethod(Method));
        }
    }


}
