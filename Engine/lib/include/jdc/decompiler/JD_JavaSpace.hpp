#pragma once
#include "../base/JD_.hpp"
#include "../base/JD_Instructions.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include "./JD_JavaMethod.hpp"
#include "./JD_JavaClass.hpp"
#include "./JD_JavaPackage.hpp"
#include <memory>
#include <string>
#include <map>

namespace jdc
{

    using xPackageMap = std::map<std::string, std::unique_ptr<xJavaPackage>>;
    using xClassMap = std::map<std::string, std::unique_ptr<xJavaClass>>;

    class xJavaSpace
    {
    public:
        xPackageMap PackageMap;
        xClassMap   ClassMap;
    };

    X_GAME_API std::string ConvertBinaryNameToPathName(const std::string & BinaryName);
    X_GAME_API std::string ConvertPathNameToBinaryName(const std::string & PathName);
    X_GAME_API std::string ConvertBinaryNameToCodeName(const std::string & BinaryName);
    X_GAME_API std::string GetSimpleClassBinaryName(const std::string & BinaryName);
    X_GAME_API std::string GetInnermostClassCodeName(const std::string & AnyTypeOfClassName);
    X_GAME_API std::string GetOutermostClassCodeName(const std::string & AnyTypeOfClassName);
    X_GAME_API std::unique_ptr<xJavaSpace> LoadJavaSpace(const std::string & RootDirectory);

}
