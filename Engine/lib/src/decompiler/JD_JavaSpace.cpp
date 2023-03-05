#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <xel/Util/Chrono.hpp>
#include <filesystem>
#include <iostream>

using namespace std;
using namespace xel;

namespace jdc
{

    std::string ConvertBinaryNameToPathName(const std::string & BinaryName)
    {
        auto Copy = BinaryName;
        #ifdef X_SYSTEM_WINDOWS
            for (auto & C : Copy) {
                if (C == '/') {
                    C = '\\';
                }
            }
        #endif
        return Copy;
    }

    std::string ConvertPathNameToBinaryName(const std::string & PathName)
    {
        auto Copy = PathName;
        #ifdef X_SYSTEM_WINDOWS
            for (auto & C : Copy) {
                if (C == '\\') {
                    C = '/';
                }
            }
        #endif
        return Copy;
    }

    std::string ConvertBinaryNameToCodeName(const std::string & BinaryName)
    {
        auto Copy = BinaryName;
        for (auto & C : Copy) {
            if (C == '/' || C == '$') {
                C = '.';
            }
        }
        return Copy;
    }

    std::string GetSimpleClassBinaryName(const std::string & BinaryName)
    {
        auto Index = BinaryName.find_last_of('/');
        if (Index == BinaryName.npos) {
            return BinaryName;
        }
        return BinaryName.substr(Index + 1);
    }

    std::string GetInnermostClassCodeName(const std::string & AnyTypeOfClassName)
    {
        auto Index = AnyTypeOfClassName.find_last_of("/$.");
        if (Index == AnyTypeOfClassName.npos) {
            return AnyTypeOfClassName;
        }
        return AnyTypeOfClassName.substr(Index + 1);
    }

    std::string GetOutermostClassCodeName(const std::string & AnyTypeOfClassName)
    {
        auto IndexStart = AnyTypeOfClassName.find_last_of('/');
        IndexStart = (IndexStart == AnyTypeOfClassName.npos ? 0 : IndexStart);
        auto IndexEnd = AnyTypeOfClassName.find_first_of("$.", IndexStart);
        size_t Count = (IndexEnd == AnyTypeOfClassName.npos ? IndexEnd : IndexEnd - IndexStart);
        return AnyTypeOfClassName.substr(0, Count);
    }

    std::unique_ptr<xJavaSpace> LoadJavaSpace(const std::string & RootDirectoryName)
    {
        auto RootDirectory = std::filesystem::path(RootDirectoryName);
        if (!std::filesystem::is_directory(RootDirectory)) {
            return {};
        }
        RootDirectory /= "./"; // force adding an extra '/' to the end of path. so sub directory path should not start with '/'
        auto NamePrefixLength = RootDirectory.string().length();

        auto JavaSpaceUPtr = std::unique_ptr<xJavaSpace>(new xJavaSpace());
        auto & PackageMap = JavaSpaceUPtr->PackageMap;
        auto & ClassMap = JavaSpaceUPtr->ClassMap;
        PackageMap.insert(std::make_pair(std::string(), std::make_unique<xJavaPackage>()));
        for(auto & Entry : std::filesystem::recursive_directory_iterator(RootDirectory)) {
            auto & Path = Entry.path();
            if (std::filesystem::is_directory(Path)) {
                auto PackagePath = Path.string().substr(NamePrefixLength);
                auto PackageBinaryName = ConvertPathNameToBinaryName(PackagePath);
                auto [Iter, _] = PackageMap.insert(std::make_pair(PackageBinaryName, std::make_unique<xJavaPackage>()));
                auto & PackageUPtr = Iter->second;
                PackageUPtr->UnfixedBinaryName = PackageBinaryName;
                PackageUPtr->UnfixedPathName   = PackagePath;
                PackageUPtr->UnfixedCodeName   = ConvertBinaryNameToCodeName(PackageBinaryName);
            }
            else {
                if (Path.extension().string() != ".class") {
                    continue;
                }
                auto FilePathString = Path.string();
                auto PackageBinaryName = ConvertPathNameToBinaryName(Path.parent_path().string().substr(NamePrefixLength));
                auto RelativePathString = FilePathString.substr(NamePrefixLength);
                auto ClassPathName = RelativePathString.substr(0, RelativePathString.length() - 6);
                auto ClassBinaryName = ConvertPathNameToBinaryName(ClassPathName);
                auto [Iter, _] = ClassMap.insert(std::make_pair(ClassBinaryName, std::make_unique<xJavaClass>()));

                auto LoadResult = LoadClassInfoFromFile(FilePathString);
                if (!LoadResult.IsOk()) {
                    cerr << "Failed to load class: " << ClassPathName << endl;
                    continue;
                }
                auto & JavaClass = *Iter->second;
                JavaClass.UnfixedPackageBinaryName = PackageBinaryName;
                JavaClass.UnfixedBinaryName = ClassBinaryName;
                JavaClass.SimpleBinaryName = GetSimpleClassBinaryName(ClassBinaryName);
                JavaClass.SimpleCodeName = ConvertBinaryNameToCodeName(JavaClass.SimpleBinaryName);
                JavaClass.InnermostCodeName = GetInnermostClassCodeName(JavaClass.SimpleCodeName);

                auto & ClassInfo = JavaClass.ClassInfo;
                ClassInfo = std::move(LoadResult.Data);
            }
        }

        for(auto & Entry : PackageMap) {
            auto & PackageUPtr = Entry.second;
            PackageUPtr->JavaSpacePtr = JavaSpaceUPtr.get();
        }

        for(auto & Entry : ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            auto & PackageUPtr = PackageMap[JavaClassUPtr->UnfixedPackageBinaryName];
            PackageUPtr->Classes.push_back(JavaClassUPtr.get());

            JavaClassUPtr->JavaSpacePtr = JavaSpaceUPtr.get();
            JavaClassUPtr->PackagePtr = PackageUPtr.get();
        }

        // FixPackagePath:
        for(auto & Entry : PackageMap) {
            auto & JavaPackageUPtr = Entry.second;
            X_DEBUG_PRINTF("Trying to find name conflicts: '%s'\n", Entry.first.c_str());

            auto ClassIter = ClassMap.find(JavaPackageUPtr->UnfixedBinaryName);
            if (ClassIter != ClassMap.end()) {
                uint64_t Counter = 0;
                while(true) {
                    auto Postfix = "_fix_" + std::to_string(Counter);
                    auto NewPackageName = JavaPackageUPtr->UnfixedBinaryName + Postfix;
                    auto NewPackageIter = PackageMap.find(NewPackageName);
                    if (NewPackageIter != PackageMap.end()) {
                        ++Counter;
                        continue;
                    }
                    auto NewClassIter = ClassMap.find(NewPackageName);
                    if (NewClassIter != ClassMap.end()) {
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
        for(auto & Entry : ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            auto & PackagePtr = JavaClassUPtr->PackagePtr;
            if (PackagePtr->FixedBinaryName.length() == PackagePtr->UnfixedBinaryName.length()) {
                JavaClassUPtr->FixedBinaryName = JavaClassUPtr->UnfixedBinaryName;
                JavaClassUPtr->FixedCodeName = ConvertBinaryNameToCodeName(JavaClassUPtr->FixedBinaryName);
                continue;
            }
            auto NewClassBinaryName = PackagePtr->FixedBinaryName + '.' + JavaClassUPtr->SimpleBinaryName;
            JavaClassUPtr->FixedBinaryName = NewClassBinaryName;
            JavaClassUPtr->FixedCodeName = ConvertBinaryNameToCodeName(NewClassBinaryName);

            X_DEBUG_PRINTF("FixClassName: %s -> %s\n", JavaClassUPtr->UnfixedBinaryName.c_str(), JavaClassUPtr->FixedBinaryName.c_str());
        }

        // extend class
        for(auto & Entry : ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            JavaClassUPtr->DoExtend();
        }

        return JavaSpaceUPtr;
    }

    std::string xJavaClass::GetFixedClassBinaryName(const std::string& OriginalClassBinaryName) const
    {
        auto & ClassMap = JavaSpacePtr->ClassMap;
        auto Iter = ClassMap.find(OriginalClassBinaryName);
        if (Iter == ClassMap.end()) { // java native class or 3rd party class
            return OriginalClassBinaryName;
        }
        return Iter->second->FixedBinaryName;
    }

    std::string xJavaClass::GetFixedClassCodeName(const std::string& OriginalClassBinaryName) const
    {
        return ConvertBinaryNameToCodeName(GetFixedClassBinaryName(OriginalClassBinaryName));
    }

    const std::string & xJavaClass::GetFixedOutermostClassBinaryName() const
    {
        auto & ClassMap = JavaSpacePtr->ClassMap;
        auto Iter = ClassMap.find(ClassInfo.GetOutermostClassBinaryName());
        assert(Iter != ClassMap.end());
        return Iter->second->FixedBinaryName;
    }

    void xJavaClass::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaClass::DoExtend %s --> %s --> %s\n", FixedBinaryName.c_str(), SimpleBinaryName.c_str(), InnermostCodeName.c_str());
        for (auto & Attribute : ClassInfo.Attributes) {
            auto & AttributeName = ClassInfo.GetConstantUtf8(Attribute.NameIndex);
            auto Reader = xStreamReader(Attribute.Binary.data());
            if (AttributeName == "SourceFile") {
                uint16_t SourceFilenameIndex = Reader.R2();
                auto & SourceFilename = ClassInfo.GetConstantUtf8(SourceFilenameIndex);
                X_DEBUG_PRINTF("SourceFilename: %s\n", SourceFilename.c_str());
                Extend.SourceFilename = SourceFilename;
                continue;
            }
            if (AttributeName == "Synthetic") {
                X_DEBUG_PRINTF("Synthetic: yes\n");
                Extend.Synthetic = true;
                continue;
            }
            if (AttributeName == "Deprecated") {
                X_DEBUG_PRINTF("Deprecated: yes\n");
                Extend.Deprecated = true;
                continue;
            }
        }

        if (Extend.SourceFilename.empty()) {
            Extend.SourceFilename = GetOutermostClassCodeName(SimpleBinaryName) + ".java";
        }

        for (size_t Index = 0 ; Index < ClassInfo.Methods.size() ; ++Index) {
            Extend.Methods.push_back(ExtractMethod(Index));
        }
    }


}
