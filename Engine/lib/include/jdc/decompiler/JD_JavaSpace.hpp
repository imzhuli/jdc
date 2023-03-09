#pragma once
#include "../base/_.hpp"
#include "./JD_JavaPackage.hpp"
#include "./JD_JavaClass.hpp"
#include <memory>
#include <string>
#include <map>

namespace jdc
{

    class xJavaPackage;
    class xJavaClass;

    using xPackageMap = std::map<std::string, std::unique_ptr<xJavaPackage>>;
    using xClassMap = std::map<std::string, std::unique_ptr<xJavaClass>>;

    class xJavaSpace
    {
    private:
        xPackageMap _PackageMap;
        xClassMap   _ClassMap;

    public:
        X_INLINE const xPackageMap &   GetPackageMap() const { return _PackageMap; }
        X_INLINE const xClassMap &     GetClassMap() const { return _ClassMap; }

        X_PRIVATE_MEMBER const std::string & GetFixedPackageBinaryName(const std::string& OriginalPackageBinaryName) const;
        X_PRIVATE_MEMBER const std::string & GetFixedClassBinaryName(const std::string& OriginalClassBinaryName) const;

    public:
        X_GAME_API_STATIC_MEMBER std::unique_ptr<xJavaSpace> LoadJavaSpace(const std::string & RootDirectory);
    };


}
