#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaType.hpp>
#include <jdc/syntax/JD_JavaPrimitiveTypes.hpp>
#include <jdc/syntax/JD_JavaObjectTypes.hpp>
#include <xel/Byte.hpp>

using namespace xel;

namespace jdc
{

    bool InitJavaSyntax()
    {
        if (!InitJavaPrimitiveTypes()) {
            return false;
        }
        auto PrimitiveTypesOnFail = xel::xScopeGuard([]{ CleanJavaPrimitiveTypes(); });

        if (!InitJavaObjectTypes()) {
            return false;
        }
        auto ObjectTypesOnFail = xel::xScopeGuard([]{ CleanJavaObjectTypes(); });

        PrimitiveTypesOnFail.Dismiss();
        ObjectTypesOnFail.Dismiss();
        return true;
    }

    void CleanJavaSyntax()
    {
        CleanJavaObjectTypes();
        CleanJavaPrimitiveTypes();
    }

    std::string ConvertBinaryNameToPathName(const std::string & BinaryName)
    {
        auto Copy = BinaryName;
        #ifdef X_SYSTEM_WINDOWS
            for (auto & C : Copy) {
                if (C == '/') {
                    C = '\\';
                }
            }
        #endif
        return Copy;
    }

    std::string ConvertPathNameToBinaryName(const std::string & PathName)
    {
        auto Copy = PathName;
        #ifdef X_SYSTEM_WINDOWS
            for (auto & C : Copy) {
                if (C == '\\') {
                    C = '/';
                }
            }
        #endif
        return Copy;
    }

    std::string ConvertBinaryNameToCodeName(const std::string & BinaryName)
    {
        auto Copy = BinaryName;
        for (auto & C : Copy) {
            if (C == '/' || C == '$') {
                C = '.';
            }
        }
        return Copy;
    }

    std::string GetSimpleClassBinaryName(const std::string & BinaryName)
    {
        auto Index = BinaryName.find_last_of('/');
        if (Index == BinaryName.npos) {
            return BinaryName;
        }
        return BinaryName.substr(Index + 1);
    }

    std::string GetInnermostClassName(const std::string & AnyTypeOfClassName)
    {
        auto Index = AnyTypeOfClassName.find_last_of("/$.");
        if (Index == AnyTypeOfClassName.npos) {
            return AnyTypeOfClassName;
        }
        return AnyTypeOfClassName.substr(Index + 1);
    }

    std::string GetOutermostClassCodeName(const std::string & AnyTypeOfClassName)
    {
        auto IndexStart = AnyTypeOfClassName.find_last_of('/');
        IndexStart = (IndexStart == AnyTypeOfClassName.npos ? 0 : IndexStart);
        auto IndexEnd = AnyTypeOfClassName.find_first_of("$.", IndexStart);
        size_t Count = (IndexEnd == AnyTypeOfClassName.npos ? IndexEnd : IndexEnd - IndexStart);
        return AnyTypeOfClassName.substr(0, Count);
    }

    static std::string ExtractTypeDescriptor(xStreamReader & Reader)
    {
        auto ArraySize = size_t();

        auto C = (char)Reader.R();
        assert(C);
        while(C == '[') {
            ++ArraySize;
            C = (char)Reader.R();
        }

        auto BaseTypeName = std::string{};
        switch(C) {
            case 'B': {
                BaseTypeName = "byte";
                break;
            }
            case 'C': {
                BaseTypeName =  "char";
                break;
            }
            case 'D': {
                BaseTypeName =  "double";
                break;
            }
            case 'F': {
                BaseTypeName =  "float";
                break;
            }
            case 'I': {
                BaseTypeName =  "int";
                break;
            }
            case 'J': {
                BaseTypeName =  "long";
                break;
            }
            case 'S': {
                BaseTypeName =  "short";
                break;
            }
            case 'Z': {
                BaseTypeName =  "boolean";
                break;
            }
            case 'V': {
                BaseTypeName =  "void";
                break;
            }
            case 'L': { // class name;
                C = (char)Reader.R();
                while(C != ';') {
                    BaseTypeName.push_back(C);
                    C = (char)Reader.R();
                }
                break;
            }
            default: {
                X_DEBUG_PRINTF("Failed to convert typename");
                Fatal("Not supported");
                return {};
            }
        }
        BaseTypeName = GetShorterBinaryName(BaseTypeName);
        while(ArraySize--) {
            BaseTypeName.append("[]");
        }
        return BaseTypeName;
    }

    std::string ConvertTypeDescriptorToBinaryName(const std::string & Descriptor)
    {
        auto Reader = xStreamReader(Descriptor.c_str());
        return ExtractTypeDescriptor(Reader);
    }

    xMethodTypeNames ConvertMethodDescriptorToBinaryNames(const std::string & Descriptor)
    {
        auto MethodTypeName = xMethodTypeNames{};

        assert(Descriptor.size());

        auto Reader = xStreamReader(Descriptor.data());
        auto C = Reader.R(); // the first MUST be '('
        assert(C == '(');
        while(true) {
            auto TestChar = (char)*static_cast<const ubyte*>(Reader);
            if (!TestChar) {
                Fatal("Should never run here");
                break;
            }
            if (TestChar == ')') { // parameter types finished
                Reader.Skip(1);
                MethodTypeName.ReturnTypeBinaryName = ExtractTypeDescriptor(Reader);
                break;
            }
            MethodTypeName.ParameterTypeBinaryNames.push_back(ExtractTypeDescriptor(Reader));
        }

        return MethodTypeName;
    }

    size_t CountMethodParameters(const std::string & Descriptor)
    {
        auto Names = ConvertMethodDescriptorToBinaryNames(Descriptor);
        return Names.ParameterTypeBinaryNames.size();
    }

    const std::string & GetShorterBinaryName(const std::string & FullBinaryName)
    {
        auto & Map = GetJavaObjectTypeMap();
        auto Iter = Map.find(FullBinaryName);
        if (Iter == Map.end()) {
            return FullBinaryName;
        }
        return Iter->second->GetSimpleBinaryName();
    }

    const std::string & GetShorterCodeName(const std::string & FullCodeName)
    {
        auto & Map = GetJavaObjectTypeCodeNameMap();
        auto Iter = Map.find(FullCodeName);
        if (Iter == Map.end()) {
            return FullCodeName;
        }
        return Iter->second->GetSimpleCodeName();
    }

}

