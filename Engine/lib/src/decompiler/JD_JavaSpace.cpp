#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <jdc/decompiler/JD_JavaPackage.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/syntax/_.hpp>
#include <filesystem>

namespace jdc
{

    std::unique_ptr<xJavaSpace> xJavaSpace::LoadJavaSpace(const std::string & RootDirectoryName)
    {
        auto RootDirectory = std::filesystem::path(RootDirectoryName);
        if (!std::filesystem::is_directory(RootDirectory)) {
            return {};
        }
        RootDirectory /= "./"; // force adding an extra '/' to the end of path. so sub directory path should not start with '/'
        auto NamePrefixLength = RootDirectory.string().length();

        auto JavaSpaceUPtr = std::unique_ptr<xJavaSpace>(new xJavaSpace());
        auto & _PackageMap = JavaSpaceUPtr->_PackageMap;
        auto & _ClassMap = JavaSpaceUPtr->_ClassMap;
        _PackageMap.insert(std::make_pair(std::string(), std::make_unique<xJavaPackage>()));
        for(auto & Entry : std::filesystem::recursive_directory_iterator(RootDirectory)) {
            auto & Path = Entry.path();
            if (std::filesystem::is_directory(Path)) {
                auto PackagePath = Path.string().substr(NamePrefixLength);
                auto PackageBinaryName = ConvertPathNameToBinaryName(PackagePath);
                auto [Iter, _] = _PackageMap.insert(std::make_pair(PackageBinaryName, std::make_unique<xJavaPackage>()));
                auto & PackageUPtr = Iter->second;
                PackageUPtr->UnfixedBinaryName = PackageBinaryName;
                PackageUPtr->UnfixedPathName   = PackagePath;
                PackageUPtr->UnfixedCodeName   = ConvertBinaryNameToCodeName(PackageBinaryName);
            }
            else {
                if (Path.extension().string() != ".class") {
                    X_DEBUG_PRINTF("Ignore non-class file: %s\n", Path.string().c_str());
                    continue;
                }
                auto FilePathString = Path.string();
                auto PackageBinaryName = ConvertPathNameToBinaryName(Path.parent_path().string().substr(NamePrefixLength));
                auto RelativePathString = FilePathString.substr(NamePrefixLength);
                auto ClassPathName = RelativePathString.substr(0, RelativePathString.length() - 6);
                auto ClassBinaryName = ConvertPathNameToBinaryName(ClassPathName);
                auto [Iter, _] = _ClassMap.insert(std::make_pair(ClassBinaryName, std::make_unique<xJavaClass>()));

                auto LoadResult = LoadClassInfoFromFile(FilePathString);
                if (!LoadResult.IsOk()) {
                    X_DEBUG_PRINTF("Failed to load class: %s\n", ClassPathName.c_str());
                    continue;
                }
                auto & JavaClass = *Iter->second;
                JavaClass._UnfixedPackageBinaryName = PackageBinaryName;
                JavaClass._UnfixedBinaryName = ClassBinaryName;
                JavaClass._SimpleBinaryName = GetSimpleClassBinaryName(ClassBinaryName);
                JavaClass._SimpleCodeName = ConvertBinaryNameToCodeName(JavaClass._SimpleBinaryName);
                JavaClass._InnermostName = GetInnermostClassName(JavaClass._SimpleCodeName);

                auto & ClassInfo = JavaClass.ClassInfo;
                ClassInfo = std::move(LoadResult.Data);
            }
        }

        for(auto & Entry : _PackageMap) {
            auto & PackageUPtr = Entry.second;
            PackageUPtr->JavaSpacePtr = JavaSpaceUPtr.get();
        }

        for(auto & Entry : _ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            auto & PackageUPtr = _PackageMap[JavaClassUPtr->_UnfixedPackageBinaryName];
            PackageUPtr->Classes.push_back(JavaClassUPtr.get());

            JavaClassUPtr->JavaSpacePtr = JavaSpaceUPtr.get();
            JavaClassUPtr->PackagePtr = PackageUPtr.get();
        }

        // FixPackagePath:
        for(auto & Entry : _PackageMap) {
            auto & JavaPackageUPtr = Entry.second;
            auto ClassIter = _ClassMap.find(JavaPackageUPtr->UnfixedBinaryName);
            if (ClassIter != _ClassMap.end()) {
                uint64_t Counter = 0;
                while(true) {
                    auto Postfix = "_fix_" + std::to_string(Counter);
                    auto NewPackageName = JavaPackageUPtr->UnfixedBinaryName + Postfix;
                    auto NewPackageIter = _PackageMap.find(NewPackageName);
                    if (NewPackageIter != _PackageMap.end()) {
                        ++Counter;
                        continue;
                    }
                    auto NewClassIter = _ClassMap.find(NewPackageName);
                    if (NewClassIter != _ClassMap.end()) {
                        ++Counter;
                        continue;
                    }
                    JavaPackageUPtr->FixedBinaryName = NewPackageName;
                    JavaPackageUPtr->FixedPathName   = JavaPackageUPtr->UnfixedPathName + Postfix;
                    JavaPackageUPtr->FixedCodeName   = JavaPackageUPtr->UnfixedCodeName + Postfix;
                    break;
                }
                X_DEBUG_PRINTF("FixPackageName: %s -> %s\n", JavaPackageUPtr->UnfixedBinaryName.c_str(), JavaPackageUPtr->FixedBinaryName.c_str());
            } else {
                JavaPackageUPtr->FixedBinaryName = JavaPackageUPtr->UnfixedBinaryName;
                JavaPackageUPtr->FixedPathName   = JavaPackageUPtr->UnfixedPathName;
                JavaPackageUPtr->FixedCodeName   = JavaPackageUPtr->UnfixedCodeName;
            }
        }

        // Fix class name with new package
        for(auto & Entry : _ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            auto & PackagePtr = JavaClassUPtr->PackagePtr;
            if (PackagePtr->FixedBinaryName.length() == PackagePtr->UnfixedBinaryName.length()) {
                JavaClassUPtr->_FixedBinaryName = JavaClassUPtr->_UnfixedBinaryName;
                JavaClassUPtr->_FixedCodeName = ConvertBinaryNameToCodeName(JavaClassUPtr->_FixedBinaryName);
                continue;
            }
            auto NewClassBinaryName = PackagePtr->FixedBinaryName + '.' + JavaClassUPtr->_SimpleBinaryName;
            JavaClassUPtr->_FixedBinaryName = NewClassBinaryName;
            JavaClassUPtr->_FixedCodeName = ConvertBinaryNameToCodeName(NewClassBinaryName);

            X_DEBUG_PRINTF("FixClassName: %s -> %s\n", JavaClassUPtr->_UnfixedBinaryName.c_str(), JavaClassUPtr->_FixedBinaryName.c_str());
        }

        // extend class
        for(auto & Entry : _ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            JavaClassUPtr->DoExtend();
        }

        return JavaSpaceUPtr;
    }

    bool xJavaSpace::BuildClassFiles()
    {
        for(auto & Entry : _ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            if (!JavaClassUPtr->DoConvert()) {
                return false;
            }
        }
        return true;
    }

    bool xJavaSpace::DumpClassFiles(const std::string & OutputDirectory)
    {
        for(auto & Entry : _ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            if (!JavaClassUPtr->DumpSourceToFile(OutputDirectory)) {
                return false;
            }
        }

        return true;
    }

}
