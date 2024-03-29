#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <jdc/decompiler/JD_JavaPackage.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/syntax/_.hpp>
#include <jdc/syntax/JD_JavaType.hpp>
#include <jdc/syntax/JD_JavaPrimitiveTypes.hpp>
#include <jdc/syntax/JD_JavaObjectTypes.hpp>
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
            auto NewClassBinaryName = PackagePtr->FixedBinaryName + '/' + JavaClassUPtr->_SimpleBinaryName;
            JavaClassUPtr->_FixedBinaryName = NewClassBinaryName;
            JavaClassUPtr->_FixedCodeName = ConvertBinaryNameToCodeName(NewClassBinaryName);

            X_DEBUG_PRINTF("FixClassName: %s -> %s\n", JavaClassUPtr->_UnfixedBinaryName.c_str(), JavaClassUPtr->_FixedBinaryName.c_str());
        }

        // extend class & build inner class chain
        for(auto & Entry : _ClassMap) {
            auto JavaClassPtr = Entry.second.get();
            JavaClassPtr->DoExtend();
        }

        // fix inner class names:
        std::vector<xJavaClass*> TryFixInnerClassList;
        size_t NextTryFixInnerClassIndex = 0;
        for(auto & Entry : _ClassMap) {
            auto JavaClassPtr = Entry.second.get();
            if (JavaClassPtr->IsInnerClass()) {
                continue;
            }
            TryFixInnerClassList.push_back(JavaClassPtr);
        }
        while(NextTryFixInnerClassIndex != TryFixInnerClassList.size()) {
            auto CurrentClassPtr = TryFixInnerClassList[NextTryFixInnerClassIndex];
            for (auto & InnerClassPtr : CurrentClassPtr->Extend.DirectInnerClasses) {
                TryFixInnerClassList.push_back(InnerClassPtr);
            }
            ++NextTryFixInnerClassIndex;
        }
        size_t FixCounter = 0;
        for (auto & InnerClassPtr : TryFixInnerClassList) {
            bool NeedCheck = false;
            std::string NewInnermostName = InnerClassPtr->_InnermostName;
            do {
                NeedCheck = false;
                for(auto AncestorClassPtr = InnerClassPtr->Extend.OuterClassPtr; AncestorClassPtr; AncestorClassPtr = AncestorClassPtr->Extend.OuterClassPtr) {
                    if (AncestorClassPtr->GetInnermostName() == NewInnermostName) {
                        NewInnermostName = InnerClassPtr->_InnermostName + "_fix_" + std::to_string(FixCounter++);
                        NeedCheck = true;
                        break;
                    }
                }
            } while(NeedCheck);
            if (NewInnermostName == InnerClassPtr->_InnermostName) {
                continue;
            }

            // auto & ParentFixedBinaryName = OuterClassPtr->GetFixedBinaryName();
            auto OuterClassPtr = InnerClassPtr->Extend.OuterClassPtr;
            InnerClassPtr->_FixedBinaryName  = OuterClassPtr->GetFixedBinaryName() + '$' + NewInnermostName;
            InnerClassPtr->_FixedCodeName    = ConvertBinaryNameToCodeName(InnerClassPtr->_FixedBinaryName);
            InnerClassPtr->_SimpleBinaryName = GetSimpleClassBinaryName(InnerClassPtr->_FixedBinaryName);
            InnerClassPtr->_SimpleCodeName   = ConvertBinaryNameToCodeName(InnerClassPtr->_SimpleBinaryName);
            InnerClassPtr->_InnermostName    = NewInnermostName;

            X_DEBUG_PRINTF("Found inner class name that needs fix: %s, new name:%s --> %s ==> %s\n", InnerClassPtr->GetUnfixedBinaryName().c_str(),
                InnerClassPtr->GetFixedBinaryName().c_str(),
                InnerClassPtr->GetSimpleBinaryName().c_str(),
                InnerClassPtr->GetSimpleCodeName().c_str()
                );
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

    xJavaType * xJavaSpace::GetJavaTypeByUnfixedBinaryName(const std::string & UnfixedBinaryName) const
    {
        auto & PrimaryTypeMap = GetJavaPrimitiveTypeMap();
        auto PrimaryIter = PrimaryTypeMap.find(UnfixedBinaryName);
        if (PrimaryIter != PrimaryTypeMap.end()) {
            return &PrimaryIter->second;
        }

        auto & DefaultObjectMap = GetJavaObjectTypeMap();
        auto ObjectIter = DefaultObjectMap.find(UnfixedBinaryName);
        if (ObjectIter != DefaultObjectMap.end()) {
            return ObjectIter->second.get();
        }

        auto & UserClassObjectMap = _ClassMap;
        auto UserClassIter = UserClassObjectMap.find(UnfixedBinaryName);
        if (UserClassIter != UserClassObjectMap.end()) {
            return UserClassIter->second.get();
        }

        return nullptr;
    }

}
