#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaType.hpp>
#include <jdc/syntax/JD_JavaPrimitiveTypes.hpp>
#include <jdc/syntax/JD_JavaObjectTypes.hpp>

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


}

