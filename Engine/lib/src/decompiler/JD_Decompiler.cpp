#include <jdc/decompiler/JD_Decompiler.hpp>
#include <jdc/decompiler/JD_JavaPackage.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaObjectTypes.hpp>
#include <xel/String.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace jdc
{

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

        if (!MakePackagePaths()) {
            return false;
        }

        if (!_JavaSpaceUPtr->BuildClassFiles()) {
            return false;
        }

        if (!_JavaSpaceUPtr->DumpClassFiles(_Config.OutputDirectory)) {
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

    void xJdc::Clean()
    {
        assert(!_JavaSpaceUPtr.get());
        assert(_OutputRootDirectory.empty());
        xel::Renew(_Config);
    }

}
