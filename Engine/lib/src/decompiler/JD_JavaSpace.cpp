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

    xJavaSpace LoadJavaSpace(const std::string & RootDirectoryName)
    {

        auto RootDirectory = std::filesystem::path(RootDirectoryName);
        if (!std::filesystem::is_directory(RootDirectory)) {
            return {};
        }
        RootDirectory /= "./"; // force adding an extra '/' to the end of path. so sub directory path should not start with '/'
        auto NamePrefixLength = RootDirectory.string().length();

        xJavaSpace JavaSpace;
        auto & PackageMap = JavaSpace.PackageMap;
        auto & ClassMap = JavaSpace.ClassMap;
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
                JavaClass.PathName = RelativePathString;
                JavaClass.BinaryName = ClassBinaryName;
                JavaClass.SimpleBinaryName = GetSimpleClassBinaryName(ClassBinaryName);
                JavaClass.CodeName = ConvertBinaryNameToCodeName(JavaClass.BinaryName);
                JavaClass.SimpleCodeName = ConvertBinaryNameToCodeName(JavaClass.SimpleBinaryName);


                auto & ClassInfo = JavaClass.ClassInfo;
                ClassInfo = std::move(LoadResult.Data);

                // TODO:
                cout << "Class: " << ClassBinaryName << endl;
                cout << "Info.CodeName: " << JavaClass.CodeName << endl;
                cout << "Info.SimpleCodeName: " << JavaClass.SimpleCodeName << endl;
            }
        }

        return JavaSpace;
    }

}
