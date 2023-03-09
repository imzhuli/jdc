#include <jdc/decompiler/JD_Decompiler.hpp>
#include <jdc/decompiler/JD_CodeMarks.hpp>
#include <jdc/syntax/_.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace jdc
{

    static std::string GeneratePackageFragment(const xJavaClass * JavaClassPtr)
    {
        auto & FixedPackageCodeName = JavaClassPtr->PackagePtr->FixedCodeName;
        assert(FixedPackageCodeName.size());

        auto ss = std::ostringstream();
        ss << CM_PACKAGE_BEGIN;
        ss << "package " << FixedPackageCodeName << ";" << std::endl;
        ss << CM_PACKAGE_END;
        return ss.str();
    }

    bool xJdc::Init(const xJdcConfig & Config)
    {
        // check:
        if (!std::filesystem::is_directory(Config.InputDirectory)) {
            X_DEBUG_PRINTF("Config.InputDirectory is not a directory\n");
            return false;
        }

        // init
        _Config = Config;

        // debug output:
        X_DEBUG_PRINTF("Config: Input:%s, Output:%s\n", Config.InputDirectory.c_str(), Config.OutputDirectory.c_str());
        return true;
    }

    bool xJdc::Execute()
    {
        _JavaSpaceUPtr = xJavaSpace::LoadJavaSpace(_Config.InputDirectory);
        if (!_JavaSpaceUPtr) {
            return false;
        }
        auto JavaSpaceCleaner = xel::xScopeGuard([this]{ _JavaSpaceUPtr.reset(); });

        _OutputRootDirectory = std::filesystem::path(_Config.OutputDirectory) / "./";
        auto OutputPathCleaner = xel::xScopeGuard([this]{ xel::Renew(_OutputRootDirectory); });

        if (!MakePackagePaths() || !MakeClassJavaFiles()) {
            return false;
        }
        return true;
    }

    bool xJdc::MakePackagePaths()
    {
        for (auto & [_, PackageUPtr] : _JavaSpaceUPtr->GetPackageMap()) {
            auto PackagePath = _OutputRootDirectory / PackageUPtr->FixedPathName;

            std::error_code Error;
            std::filesystem::create_directories(PackagePath, Error);
            if (Error) {
                X_DEBUG_PRINTF("Failed to create package: %s\n", PackagePath.string().c_str());
                return false;
            }
        }
        return true;
    }

    bool xJdc::MakeClassJavaFiles()
    {
        for (auto & [_, UPtr] : _JavaSpaceUPtr->GetClassMap()) {
            auto ClassPtr = UPtr.get();
            if (ClassPtr->IsInnerClass()) {
                X_DEBUG_PRINTF("Ignore inner class: %s\n", ClassPtr->GetFixedBinaryName().c_str());
                continue;
            }
            if (ClassPtr->IsSynthetic()) {
                X_DEBUG_PRINTF("Ignore synthetic class: %s\n", ClassPtr->GetFixedBinaryName().c_str());
                continue;
            }

            auto ClassPath = _OutputRootDirectory / (ClassPtr->GetFixedBinaryName() + ".java");
            X_DEBUG_PRINTF("Create java class file: %s\n", ClassPath.string().c_str());

            auto File = std::ofstream(ClassPath, std::ios_base::binary | std::ios_base::trunc);
            if (!File) {
                X_DEBUG_PRINTF("Failed to create and truncate file: %s\n", ClassPath.string().c_str());
                return false;
            }
            auto FragmentString = GeneratePackageFragment(ClassPtr);
            if (FragmentString.empty()) {
                X_DEBUG_PRINTF("Failed to generate package fragment for class: %s\n", ClassPath.string().c_str());
                return false;
            }
            File << FragmentString << std::endl;
        }
        return true;
    }

    void xJdc::Clean()
    {

    }

}
