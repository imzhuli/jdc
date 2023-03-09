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
        xPackageMap PackageMap;
        xClassMap   ClassMap;

    public:
        X_PRIVATE_MEMBER std::string GetFixedClassBinaryName(const std::string& OriginalClassBinaryName) const;

    public:
        X_GAME_API_STATIC_MEMBER std::unique_ptr<xJavaSpace> LoadJavaSpace(const std::string & RootDirectory);
    };


}
