#include <jdc/base/JD_Instructions.hpp>
#include <jdc/decompiler/JD_CodeGenerator.hpp>
#include <jdc/class_file/JD_Attribute.hpp>
#include <xel/String.hpp>
#include <sstream>
#include <iostream>

using namespace xel;
using namespace std;

namespace jdc
{


    void xJavaMethod::Decode()
    {
        // read Attribute:

        DecodeNameStrings();
        Decode_Round_1();
    }

    void xJavaMethod::DecodeNameStrings()
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

    void xJavaMethod::Decode_Round_1()
    {

    }

    std::string xJavaMethod::GetQualifiedName()
    {
        return QualifierString + ' ' + GetUnqualifiedName();
    }

    std::string xJavaMethod::GetUnqualifiedName()
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