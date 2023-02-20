#pragma once
#include "../base/JD_Base.hpp"
#include "../jvm/JD_ClassInfo.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace jdc
{

    class xJavaPackage;
    class xJavaClass;

    class xJavaPackage
    {
    public:
        std::string BinaryName;
        std::string PathName;
        std::string CodeName;
        std::vector<xJavaClass*> Classes;
    };

    class xJavaClass
    {
    public:
        xJavaPackage * PackagePtr;

        std::string PackageBinaryName;
        std::string PathName;
        std::string BinaryName;
        std::string SimpleBinaryName;
        std::string CodeName;
        std::string SimpleCodeName;
        xClassInfo ClassInfo;

        struct {
            std::string SourceFilename;


        } Extend;

        X_INLINE const std::string & GetPackageBinaryName() const { return PackagePtr->BinaryName; }
        X_INLINE const std::string & GetPackagePathName() const { return PackagePtr->PathName; }
        X_INLINE const std::string & GetPackageCodeName() const { return PackagePtr->CodeName; }
    };

    using xPackageMap = std::unordered_map<std::string, std::unique_ptr<xJavaPackage>>;
    using xClassMap = std::unordered_map<std::string, std::unique_ptr<xJavaClass>>;

    class xJavaSpace
    {
    public:
        xPackageMap PackageMap;
        xClassMap   ClassMap;
    };

    X_GAME_API std::string ConvertPathNameToBinaryName(const std::string & PathName);
    X_GAME_API std::string ConvertBinaryNameToCodeName(const std::string & BinaryName);
    X_GAME_API std::string GetSimpleClassBinaryName(const std::string & BinaryName);
    X_GAME_API xJavaSpace LoadJavaSpace(const std::string & RootDirectory);

}
