#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaPackage.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaType.hpp>
#include <xel/Byte.hpp>

using namespace xel;

namespace jdc
{

    std::string xJavaClass::GetFixedClassBinaryName(const std::string& OriginalClassBinaryName) const
    {
        auto & ClassMap = JavaSpacePtr->ClassMap;
        auto Iter = ClassMap.find(OriginalClassBinaryName);
        if (Iter == ClassMap.end()) { // java native class or 3rd party class
            return OriginalClassBinaryName;
        }
        return Iter->second->FixedBinaryName;
    }

    std::string xJavaClass::GetFixedClassCodeName(const std::string& OriginalClassBinaryName) const
    {
        return ConvertBinaryNameToCodeName(GetFixedClassBinaryName(OriginalClassBinaryName));
    }

    const std::string & xJavaClass::GetFixedOutermostClassBinaryName() const
    {
        auto & ClassMap = JavaSpacePtr->ClassMap;
        auto Iter = ClassMap.find(ClassInfo.GetOutermostClassBinaryName());
        assert(Iter != ClassMap.end());
        return Iter->second->FixedBinaryName;
    }

    xJavaMethod xJavaClass::ExtractMethod(size_t Index)
    {
        auto Method = xJavaMethod();
        /*
        auto & MethodInfo = ClassInfo.Methods[Index];
        auto MethodName = ClassInfo.GetConstantUtf8(MethodInfo.NameIndex);
        Method.ClassInfoPtr = &ClassInfo;
        Method.MethodInfoPtr = &MethodInfo;
        Method.OriginalNameView = MethodName;

        // build method identifier:
        if (MethodName == "<clinit>") {
            Method.Identifier = "static";
        }
        else if (MethodName == "<init>") {
            Method.Identifier = InnermostCodeName;
        }
        else {
            Method.Identifier = MethodName;
        }

        for (auto & Attribute : MethodInfo.Attributes) {
            auto & AttributeName = ClassInfo.GetConstantUtf8(Attribute.NameIndex);
            X_DEBUG_PRINTF("MethodInfo.Attribute: %s\n", AttributeName.c_str());
            if (AttributeName == "Code") {
                Method.AttributeCode.Extract(Attribute.Binary);
                continue;
            }
            if (AttributeName == "MethodParameters") {
                Method.AttributeParameters.Extract(Attribute.Binary, &ClassInfo);
            }
        }

        do { // extract Param types

        } while(false);

        // decode:
        Method.Decode();
        X_DEBUG_PRINTF("DecodedMethod: %s  flags=%u\n", Method.GetQualifiedName().c_str(), (uint)Method.MethodInfoPtr->AccessFlags);

        */
        return Method;
    }


    void xJavaClass::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaClass::DoExtend %s --> %s --> %s\n", FixedBinaryName.c_str(), SimpleBinaryName.c_str(), InnermostCodeName.c_str());

        for (auto & Attribute : ClassInfo.Attributes) {
            auto & AttributeName = ClassInfo.GetConstantUtf8(Attribute.NameIndex);
            auto & AttributeBinary = Attribute.Binary;
            if (AttributeName == "Deprecated") {
                X_DEBUG_PRINTF("Deprecated: yes\n");
                Extend.AttributeDeprecated.Extract(AttributeBinary);
                continue;
            }
            if (AttributeName == "Synthetic") {
                X_DEBUG_PRINTF("Synthetic: yes\n");
                Extend.AttributeSynthetic.Extract(AttributeBinary);
                continue;
            }
            if (AttributeName == "InnerClasses") {
                Extend.AttributeInnerClasses.Extract(AttributeBinary);
                continue;
            }
            if (AttributeName == "SourceFile") {
                Extend.AttributeSourceFile.Extract(AttributeBinary, &ClassInfo);
                continue;
            }
            if (AttributeName == "BootstrapMethods") {
                Extend.AttributeBootstrapMethods.Extract(AttributeBinary, &ClassInfo);
                continue;
            }
        }

        if (Extend.AttributeSourceFile.SourceFile.empty()) {
            Extend.AttributeSourceFile.SourceFile = GetOutermostClassCodeName(SimpleBinaryName) + ".java";
        }

        for (size_t Index = 0 ; Index < ClassInfo.Methods.size() ; ++Index) {
            Extend.Methods.push_back(ExtractMethod(Index));
        }
    }


}
