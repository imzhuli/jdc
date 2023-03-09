#pragma once
#include "../base/_.hpp"
#include <string>
#include <map>

namespace jdc
{

    X_PRIVATE bool InitJavaSyntax();
    X_PRIVATE void CleanJavaSyntax();

    X_PRIVATE std::string ConvertBinaryNameToPathName(const std::string & BinaryName);
    X_PRIVATE std::string ConvertPathNameToBinaryName(const std::string & PathName);
    X_PRIVATE std::string ConvertBinaryNameToCodeName(const std::string & BinaryName);
    X_PRIVATE std::string GetSimpleClassBinaryName(const std::string & BinaryName);
    X_PRIVATE std::string GetInnermostClassCodeName(const std::string & AnyTypeOfClassName);
    X_PRIVATE std::string GetOutermostClassCodeName(const std::string & AnyTypeOfClassName);

}
