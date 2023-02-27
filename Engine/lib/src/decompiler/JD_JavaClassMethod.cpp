#include <jdc/decompiler/JD_Instructions.hpp>
#include <jdc/decompiler/JD_CodeGenerator.hpp>
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

        const ubyte * CodeAttribute = nullptr;
        size_t CodeLength = 0;
        for (auto & Attribute : MethodInfo.Attributes) {
            auto & AttributeName = ClassInfo.GetConstantUtf8(Attribute.NameIndex);
            if (AttributeName == "Code") {
                CodeAttribute = (const ubyte*)Attribute.Binary.data();
                CodeLength = Attribute.Binary.size();
                continue;
            }
        }

        // build method identifier:
        if (MethodName == "<clinit>") {
            Method.Identifier = "static";
        }
        else if (MethodName == "<init>") {
            Method.Identifier = SimpleCodeName;
        }

        do { // extract some marks:
            if (MethodInfo.AccessFlags & ACC_SYNTHETIC) {
                Method.Synthetic = true;
            }
        } while(false);

        X_DEBUG_PRINTF("MethodName: %s, %" PRIx32 "\n", MethodName.c_str(), (uint32_t)MethodInfo.AccessFlags);
        do { // extract Param types
            auto & Descriptor = ClassInfo.GetConstantUtf8(MethodInfo.DescriptorIndex);
            auto TypeBinaryNames = ClassInfo.ExtractTypeBinaryNames(Descriptor);
            for (auto & Name : TypeBinaryNames) {
                X_DEBUG_PRINTF("  Type: %s\n", Name.c_str());
            }

        } while(false);

        if (Method.Synthetic) {
            return Method;
        }

        // decode:
        if (!CodeAttribute || !CodeLength) {
            X_DEBUG_PRINTF("NoCode\n");
            return Method;
        }



        return Method;
    }


}