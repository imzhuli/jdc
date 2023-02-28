#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <filesystem>
#include <iostream>

using namespace std;
using namespace xel;

namespace jdc
{

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
                auto PackageBinaryName = ConvertPathNameToBinaryName(Path.string().substr(NamePrefixLength));
                PackageMap.insert(std::make_pair(PackageBinaryName, std::make_unique<xJavaPackage>()));

                cout << "Package: " << PackageBinaryName << endl;
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
                JavaClass.PackageBinaryName = PackageBinaryName;
                JavaClass.PathName = RelativePathString;
                JavaClass.BinaryName = ClassBinaryName;
                JavaClass.SimpleBinaryName = GetSimpleClassBinaryName(ClassBinaryName);
                JavaClass.CodeName = ConvertBinaryNameToCodeName(JavaClass.BinaryName);
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
            auto & PackageUPtr = PackageMap[JavaClassUPtr->PackageBinaryName];
            PackageUPtr->Classes.push_back(JavaClassUPtr.get());

            JavaClassUPtr->JavaSpacePtr = JavaSpaceUPtr.get();
            JavaClassUPtr->PackagePtr = PackageUPtr.get();
        }

        for(auto & Entry : ClassMap) {
            auto & JavaClassUPtr = Entry.second;
            JavaClassUPtr->DoExtend();
        }

        return JavaSpaceUPtr;
    }

    void xJavaClass::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaClass::DoExtend %s --> %s --> %s\n", BinaryName.c_str(), SimpleBinaryName.c_str(), InnermostCodeName.c_str());
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

        for (size_t Index = 0 ; Index < ClassInfo.Methods.size() ; ++Index) {
            Extend.Methods.push_back(ExtractMethod(Index));
        }
    }


}
