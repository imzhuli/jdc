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
    class xJavaSyntaxTree;

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

        X_INLINE xJavaPackage * GetPackage(const std::string & UnfixedPackageBinaryName) const {
            auto Iter = _PackageMap.find(UnfixedPackageBinaryName);
            return (Iter == _PackageMap.end()) ? nullptr : Iter->second.get();
        }
        X_INLINE xJavaClass * GetClass(const std::string & UnfixedClassBinaryName) const {
            auto Iter = _ClassMap.find(UnfixedClassBinaryName);
            return (Iter == _ClassMap.end()) ? nullptr : Iter->second.get();
        }
        X_INLINE std::string GetFixedClassBinaryName(const std::string & UnfixedClassBinaryName) const {
            auto JavaClassPtr = GetClass(UnfixedClassBinaryName);
            if (JavaClassPtr) {
                return JavaClassPtr->GetFixedBinaryName();
            }
            return UnfixedClassBinaryName;
        }
        X_INLINE std::string GetFixedClassCodeName(const std::string & UnfixedClassBinaryName) const {
            auto JavaTypePtr = GetJavaTypeByUnfixedBinaryName(UnfixedClassBinaryName);
            if(!JavaTypePtr) { // maybe from 3rd package
                return ConvertBinaryNameToCodeName(UnfixedClassBinaryName);
            }
            if (!JavaTypePtr->IsLangType()) {
                return JavaTypePtr->GetFixedCodeName();
            }
            return JavaTypePtr->GetSimpleCodeName();
        }

    public:
        X_GAME_API_STATIC_MEMBER std::unique_ptr<xJavaSpace> LoadJavaSpace(const std::string & RootDirectory);

        X_PRIVATE_MEMBER bool BuildClassFiles();
        X_PRIVATE_MEMBER bool DumpClassFiles(const std::string & OutputDirectory);

        X_PRIVATE_MEMBER xJavaType * GetJavaTypeByUnfixedBinaryName(const std::string & UnfixedBinaryName) const;
    };


}
