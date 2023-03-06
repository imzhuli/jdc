#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
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
        auto & MethodInfo = ClassInfo.Methods[Index];
        auto MethodName = ClassInfo.GetConstantUtf8(MethodInfo.NameIndex);
        xJavaMethod Method;
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
            if (AttributeName == "Code") {
                Method.CodeAttribute.Extract(Attribute.Binary);
                continue;
            }
        }

        do { // extract Param types

        } while(false);

        // decode:
        Method.Decode();
        X_DEBUG_PRINTF("DecodedMethod: %s  flags=%u\n", Method.GetQualifiedName().c_str(), (uint)Method.MethodInfoPtr->AccessFlags);

        return Method;
    }


    void xJavaClass::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaClass::DoExtend %s --> %s --> %s\n", FixedBinaryName.c_str(), SimpleBinaryName.c_str(), InnermostCodeName.c_str());

        for (auto & Attribute : ClassInfo.Attributes) {
            auto & AttributeName = ClassInfo.GetConstantUtf8(Attribute.NameIndex);
            auto Reader = xStreamReader(Attribute.Binary.data());
            if (AttributeName == "SourceFile") {
                uint16_t SourceFilenameIndex = Reader.R2();
                auto & SourceFilename = ClassInfo.GetConstantUtf8(SourceFilenameIndex);
                X_DEBUG_PRINTF("SourceFilename: %s\n", SourceFilename.c_str());
                Extend.SourceFilename = SourceFilename;
                continue;
            }
            if (AttributeName == "Synthetic") {
                X_DEBUG_PRINTF("Synthetic: yes\n");
                Extend.Synthetic = true;
                continue;
            }
            if (AttributeName == "Deprecated") {
                X_DEBUG_PRINTF("Deprecated: yes\n");
                Extend.Deprecated = true;
                continue;
            }
            if (AttributeName == "InnerClasses") {
                // Oracle's Java Virtual Machine implementation DOES NOT check the consistency of an InnerClasses attribute
                // against a class file representing a class or interface referenced by the attribute.
                // so, just ignore this attribute
                size_t Total = Reader.R2();
                for (size_t Index = 0 ; Index < Total; ++Index) {
                #ifndef NDEBUG
                    uint16_t InnerClassInfoIndex = Reader.R2();
                    uint16_t OuterClassInfoIndex = Reader.R2();
                    uint16_t InnerBinaryNameIndex = Reader.R2();
                    uint16_t InnerAccessFlags = Reader.R2();

                    auto & InnerClassName = InnerClassInfoIndex ? ClassInfo.GetConstantClassBinaryName(InnerClassInfoIndex) : std::string();
                    auto & OuterClassName = OuterClassInfoIndex ? ClassInfo.GetConstantClassBinaryName(OuterClassInfoIndex) : std::string();
                    auto & InnerBinaryName = InnerBinaryNameIndex ? ClassInfo.GetConstantUtf8(InnerBinaryNameIndex) : std::string();

                    X_DEBUG_PRINTF("InnerClasses: %s -> %s -> %s\n", InnerClassName.c_str(), OuterClassName.c_str(), InnerBinaryName.c_str());

                    (void)InnerClassInfoIndex;
                    (void)OuterClassInfoIndex;
                    (void)InnerBinaryNameIndex;
                    (void)InnerAccessFlags;
                #endif
                }
                continue;
            }
        }

        if (Extend.SourceFilename.empty()) {
            Extend.SourceFilename = GetOutermostClassCodeName(SimpleBinaryName) + ".java";
        }

        for (size_t Index = 0 ; Index < ClassInfo.Methods.size() ; ++Index) {
            Extend.Methods.push_back(ExtractMethod(Index));
        }
    }


}
