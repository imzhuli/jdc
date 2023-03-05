#pragma once
#include "../base/JD_Base.hpp"
#include "../base/JD_Instructions.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include "./JD_Operand.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace jdc
{

    class xJavaPackage;
    class xJavaClass;
    class xJavaSpace;

    class xJavaPackage
    {
    public:
        xJavaSpace * JavaSpacePtr = nullptr;

        std::string UnfixedBinaryName;
        std::string UnfixedPathName;
        std::string UnfixedCodeName;

        std::string FixedBinaryName;
        std::string FixedPathName;
        std::string FixedCodeName;

        std::vector<xJavaClass*> Classes;
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

        std::string UnfixedPackageBinaryName;
        std::string UnfixedBinaryName;
        std::string FixedBinaryName;
        std::string FixedCodeName;
        std::string SimpleBinaryName;
        std::string SimpleCodeName;
        std::string InnermostCodeName;
        xClassInfo  ClassInfo;

        struct {
            std::string             SourceFilename;
            bool                    Synthetic = false;
            bool                    Deprecated = false;
            std::vector<xJavaMethod>    Methods;
        } Extend;

        X_INLINE const std::string & GetFixedPackageBinaryName() const { return PackagePtr->FixedBinaryName; }
        X_INLINE const std::string & GetFixedPackagePathName() const { return PackagePtr->FixedPathName; }
        X_INLINE const std::string & GetFixedPackageCodeName() const { return PackagePtr->FixedCodeName; }
        X_INLINE bool IsInnerClass() const { return SimpleCodeName.length() != InnermostCodeName.length(); }

        X_GAME_API_MEMBER std::string GetFixedClassBinaryName(const std::string& OriginalClassBinaryName) const;
        X_GAME_API_MEMBER std::string GetFixedClassCodeName(const std::string& OriginalClassBinaryName) const;
        X_GAME_API_MEMBER const std::string & GetFixedOutermostClassBinaryName() const;
        X_GAME_API_MEMBER void DoExtend();
        X_GAME_API_MEMBER xJavaMethod ExtractMethod(size_t Index);

    };

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
