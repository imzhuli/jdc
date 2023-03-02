#include <jdc/decompiler/JD_Instructions.hpp>
#include <jdc/decompiler/JD_CodeGenerator.hpp>
#include <xel/String.hpp>
#include <sstream>
#include <iostream>

using namespace xel;
using namespace std;

namespace jdc
{

    xMethod xJavaClass::ExtractMethod(size_t Index)
    {
        auto & MethodInfo = ClassInfo.Methods[Index];
        auto MethodName = ClassInfo.GetConstantUtf8(MethodInfo.NameIndex);
        xMethod Method;
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
                Method.CodeBinaryView = { Attribute.Binary.data(), Attribute.Binary.size() };
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

    void xMethod::Decode()
    {
        DecodeNameStrings();
        Decode_Round_1();
    }

    void xMethod::DecodeNameStrings()
    {
        // typenames
        auto & Descriptor = ClassInfoPtr->GetConstantUtf8(MethodInfoPtr->DescriptorIndex);
        TypeBinaryNames = ClassInfoPtr->ExtractTypeBinaryNames(Descriptor);

        if (Identifier != "static") {
            auto AccessFlags = MethodInfoPtr->AccessFlags;
            std::vector<std::string> Qualifiers;
            if (AccessFlags & ACC_PUBLIC) {
                Qualifiers.push_back("public");
            } else if(AccessFlags & ACC_PRIVATE) {
                Qualifiers.push_back("private");
            } else if(AccessFlags & ACC_PROTECTED) {
                Qualifiers.push_back("protected");
            }

            if (AccessFlags & ACC_STATIC) {
                Qualifiers.push_back("static");
            }

            if (AccessFlags & ACC_FINAL) {
                Qualifiers.push_back("final");
            }
            QualifierString = JoinStr(Qualifiers.begin(), Qualifiers.end(), ' ');
        }
    }

    void xMethod::Decode_Round_1()
    {

    }

    std::string xMethod::GetQualifiedName()
    {
        return QualifierString + ' ' + GetUnqualifiedName();
    }

    std::string xMethod::GetUnqualifiedName()
    {
        size_t ParamNumber = GetParamNumber();

        std::ostringstream ss;
        ss << GetReturnTypeBinaryName() << ' ' << Identifier << '(' ;
        if (ParamNumber) {
            for (size_t i = 0 ; i < ParamNumber - 1; ++i) {
                ss << GetParamTypeBinaryName(i) << ", ";
            }
            ss << GetReturnTypeBinaryName();
        }
        ss << ')';
        return ss.str();
    }

}