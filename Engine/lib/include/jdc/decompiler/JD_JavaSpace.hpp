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
    class xJavaSpace;

    class xJavaPackage
    {
    public:
        xJavaSpace * JavaSpacePtr = nullptr;

        std::string BinaryName;
        std::string PathName;
        std::string CodeName;
        std::vector<xJavaClass*> Classes;
    };

    struct xJavaElementValue
    {

    };

    struct xJavaAnnotation
    {
        std::string BinaryName;
    };

    class xJavaClass
    {
    public:
        xJavaSpace * JavaSpacePtr = nullptr;
        xJavaPackage * PackagePtr = nullptr;

        std::string PackageBinaryName;
        std::string PathName;
        std::string BinaryName;
        std::string SimpleBinaryName;
        std::string CodeName;
        std::string SimpleCodeName;
        xClassInfo  ClassInfo;

        struct {
            std::string SourceFilename;
            bool        Synthetic = false;
            bool        Deprecated = false;


        } Extend;

        X_INLINE const std::string & GetPackageBinaryName() const { return PackagePtr->BinaryName; }
        X_INLINE const std::string & GetPackagePathName() const { return PackagePtr->PathName; }
        X_INLINE const std::string & GetPackageCodeName() const { return PackagePtr->CodeName; }

        X_GAME_API_MEMBER void DoExtend();
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
    X_GAME_API std::unique_ptr<xJavaSpace> LoadJavaSpace(const std::string & RootDirectory);

}
