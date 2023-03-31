#pragma once
#include "../base/_.hpp"
#include <string>
#include <vector>
namespace jdc
{

    X_PRIVATE bool InitJavaSyntax();
    X_PRIVATE void CleanJavaSyntax();

    X_PRIVATE std::string ConvertBinaryNameToPathName(const std::string & BinaryName);
    X_PRIVATE std::string ConvertPathNameToBinaryName(const std::string & PathName);
    X_PRIVATE std::string ConvertBinaryNameToCodeName(const std::string & BinaryName);
    X_PRIVATE std::string GetSimpleClassBinaryName(const std::string & BinaryName);
    X_PRIVATE std::string GetInnermostClassName(const std::string & AnyTypeOfClassName);
    X_PRIVATE std::string GetOutermostClassCodeName(const std::string & AnyTypeOfClassName);

    X_PRIVATE const std::string & GetShorterBinaryName(const std::string & FullBinaryName);
    X_PRIVATE const std::string & GetShorterCodeName(const std::string & FullCodeName);

    struct xMethodTypeNames {
        std::string              ReturnTypeBinaryName;
        std::vector<std::string> ParameterTypeBinaryNames;
    };

    X_PRIVATE std::string ConvertTypeDescriptorToBinaryName(const std::string & Descriptor);
    X_PRIVATE xMethodTypeNames ConvertMethodDescriptorToBinaryNames(const std::string & Descriptor);


}
