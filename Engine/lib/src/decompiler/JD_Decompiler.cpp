#include <jdc/decompiler/JD_Decompiler.hpp>
#include <jdc/decompiler/JD_JavaPackage.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_CodeMarks.hpp>
#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaObjectTypes.hpp>
#include <xel/String.hpp>
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

    static std::string GenerateClassFrameFragment(const xJavaClass * JavaClassPtr)
    {
        auto ss = std::ostringstream();

        // TODO: Annotations:
        auto & UnfixedBinaryName = JavaClassPtr->GetUnfixedBinaryName();

        // Class:
        std::vector<std::string> ClassIdentity;
        do { // qualifiers:
            auto QualifierParts = std::vector<std::string>();
            auto Flags = JavaClassPtr->ClassInfo.AccessFlags;
            if (Flags & ACC_PUBLIC) {
                QualifierParts.push_back("public");
            }
            if (Flags & ACC_PRIVATE) {
                QualifierParts.push_back("private");
            }
            if (Flags & ACC_PROTECTED) {
                QualifierParts.push_back("protected");
            }

            if (Flags & ACC_STATIC) {
                QualifierParts.push_back("static");
            }

            if (Flags & ACC_ABSTRACT) {
                QualifierParts.push_back("abstract");
            }
            if (Flags & ACC_FINAL) {
                QualifierParts.push_back("final");
            }

            if (QualifierParts.size()) {
                ClassIdentity.push_back(xel::JoinStr(QualifierParts, ' '));
            }
        } while(false);

        if (JavaClassPtr->IsEnum()) {
            ClassIdentity.push_back("enum");
            ClassIdentity.push_back(JavaClassPtr->GetSimpleCodeName());
        } else if (JavaClassPtr->IsAnnotation()) {
            ClassIdentity.push_back("@interface");
            ClassIdentity.push_back(JavaClassPtr->GetSimpleCodeName());

            auto SuperInterfaceCodeNames = std::vector<std::string>();
            auto SuperInterfaceBinaryNames = JavaClassPtr->GetUnfixedInterfaceBinaryNames();
            for (auto & SuperInterfaceName : SuperInterfaceBinaryNames) {
                auto SuperClassPtr = JavaClassPtr->JavaSpacePtr->GetClass(SuperInterfaceName);
                if (SuperClassPtr) {
                    SuperInterfaceCodeNames.push_back(SuperClassPtr->GetFixedCodeName());
                } else { // maybe from non-jre-lang libs
                    SuperInterfaceCodeNames.push_back(ConvertBinaryNameToCodeName(SuperInterfaceName));
                }
            }
            if (SuperInterfaceCodeNames.size()) {
                ClassIdentity.push_back("extends");
                ClassIdentity.push_back(xel::JoinStr(SuperInterfaceCodeNames, ","));
            }
        } else if (JavaClassPtr->IsInterface()) {
            ClassIdentity.push_back("interface");
            ClassIdentity.push_back(JavaClassPtr->GetSimpleCodeName());
            // TODO: super interfaces
            auto SuperInterfaceCodeNames = std::vector<std::string>();
            auto SuperInterfaceBinaryNames = JavaClassPtr->GetUnfixedInterfaceBinaryNames();
            for (auto & SuperInterfaceName : SuperInterfaceBinaryNames) {
                auto SuperClassPtr = JavaClassPtr->JavaSpacePtr->GetClass(SuperInterfaceName);
                if (SuperClassPtr) {
                    SuperInterfaceCodeNames.push_back(SuperClassPtr->GetFixedCodeName());
                } else { // maybe from non-jre-lang libs
                    SuperInterfaceCodeNames.push_back(ConvertBinaryNameToCodeName(SuperInterfaceName));
                }
            }
        } else {
            ClassIdentity.push_back("class");
            ClassIdentity.push_back(JavaClassPtr->GetSimpleCodeName());
            auto & SuperClassName = JavaClassPtr->GetUnfixedSuperClassBinaryName();
            if (!xJavaObjectType::IsDefaultClassBase(SuperClassName)) {
                auto SuperClassPtr = JavaClassPtr->JavaSpacePtr->GetClass(SuperClassName);
                if (SuperClassPtr) {
                    ClassIdentity.push_back("extends " + SuperClassPtr->GetFixedCodeName());
                } else { // maybe from non-jre-lang libs
                    ClassIdentity.push_back("extends " + ConvertBinaryNameToCodeName(SuperClassName));
                }
            }

            auto SuperInterfaceCodeNames = std::vector<std::string>();
            auto SuperInterfaceBinaryNames = JavaClassPtr->GetUnfixedInterfaceBinaryNames();
            for (auto & SuperInterfaceName : SuperInterfaceBinaryNames) {
                auto SuperClassPtr = JavaClassPtr->JavaSpacePtr->GetClass(SuperInterfaceName);
                if (SuperClassPtr) {
                    SuperInterfaceCodeNames.push_back(SuperClassPtr->GetFixedCodeName());
                } else { // maybe from non-jre-lang libs
                    SuperInterfaceCodeNames.push_back(ConvertBinaryNameToCodeName(SuperInterfaceName));
                }
            }
            if (SuperInterfaceCodeNames.size()) {
                ClassIdentity.push_back("implements");
                ClassIdentity.push_back(xel::JoinStr(SuperInterfaceCodeNames, ","));
            }
        }

        ss << CM_CLASS_IDENTIFIER << UnfixedBinaryName << CM_ENDLINE;
        ss << xel::JoinStr(ClassIdentity, ' ');
        ss << CM_ENDLINE;
        ss << "{" << CM_ENDLINE;
        ss << CM_CLASS_BODY_BEGIN << UnfixedBinaryName << CM_ENDLINE;
        ss << CM_CLASS_BODY_END << UnfixedBinaryName << CM_ENDLINE;
        ss << "}";
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

        if (!MakePackagePaths()) {
            return false;
        }

        // if (!MakeClassJavaFiles()) {
        //     return false;
        // }

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

    bool xJdc::MakeClassJavaFiles()
    {
        auto JavaSpacePtr = _JavaSpaceUPtr.get();
        for (auto & [_, UPtr] : JavaSpacePtr->GetClassMap()) {
            auto ClassPtr = UPtr.get();
            if (ClassPtr->IsInnerClass()) {
                X_DEBUG_PRINTF("Ignore inner class: %s\n", ClassPtr->GetFixedBinaryName().c_str());
                continue;
            }
            if (ClassPtr->IsSynthetic()) {
                X_DEBUG_PRINTF("Ignore synthetic class: %s\n", ClassPtr->GetFixedBinaryName().c_str());
                continue;
            }

            auto PackagePath = _OutputRootDirectory / JavaSpacePtr->GetPackage(ClassPtr->GetUnfixedPackageBinaryName())->FixedPathName;
            auto ClassPath = PackagePath / (ClassPtr->GetSourceFilename() + ".java");
            X_DEBUG_PRINTF("Create java class file: %s\n", ClassPath.string().c_str());

            auto File = std::ofstream(ClassPath, std::ios_base::binary | std::ios_base::trunc);
            if (!File) {
                X_DEBUG_PRINTF("Failed to create and truncate file: %s\n", ClassPath.string().c_str());
                return false;
            }

            auto PackageFragmentString = GeneratePackageFragment(ClassPtr);
            if (PackageFragmentString.empty()) {
                X_DEBUG_PRINTF("Failed to generate package fragment for class: %s\n", ClassPath.string().c_str());
                return false;
            }
            File << PackageFragmentString << std::endl;
            auto AutoEndFile = xel::xScopeGuard([&]{ File << CM_CLASS_FILE_END << std::endl; });

            if (!ClassPtr->IsMainClass()) {
                continue;
            }

            auto ClassFramgmentString = GenerateClassFrameFragment(ClassPtr);
            File << ClassFramgmentString << std::endl;
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
