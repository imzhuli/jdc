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

        X_INLINE const xJavaPackage * GetPackage(const std::string & UnfixedPackageBinaryName) const {
            auto Iter = _PackageMap.find(UnfixedPackageBinaryName);
            return (Iter == _PackageMap.end()) ? nullptr : Iter->second.get();
        }
        X_INLINE const xJavaClass * GetClass(const std::string & UnfixedClassBinaryName) const {
            auto Iter = _ClassMap.find(UnfixedClassBinaryName);
            return (Iter == _ClassMap.end()) ? nullptr : Iter->second.get();
        }

    public:
        X_GAME_API_STATIC_MEMBER std::unique_ptr<xJavaSpace> LoadJavaSpace(const std::string & RootDirectory);
    };


}
