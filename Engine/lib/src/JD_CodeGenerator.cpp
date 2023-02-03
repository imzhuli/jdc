#include <jdc/JD_CodeGenerator.hpp>
#include <jdc/JD_Util.hpp>

using namespace std;
using namespace xel;

namespace jdc
{

    static const std::string JavaDefaultRootClassPathName = "java/lang/Object";

    std::string GetPackageName(const std::string & ClassPathName)
    {
        auto IndexIter = ClassPathName.find_last_of('/');
        auto PackageName = ClassPathName.substr(0, IndexIter);
        for (auto & C : PackageName) {
            if (C == '/') {
                C = '.';
            }
        }
        return PackageName;
    }

    std::string GetFullClassName(const std::string & ClassPathName)
    {
        auto Copy = ClassPathName;
        for (auto & C : Copy) {
            if (C == '/') {
                C = '.';
            }
        }
        return Copy;
    }

    std::string GetClassName(const std::string & ClassPathName)
    {
        auto IndexIter = ClassPathName.find_last_of('/');
        if (IndexIter == ClassPathName.npos) {
            return ClassPathName;
        }
        return ClassPathName.substr(IndexIter + 1);
    }

    std::pair<std::string, std::string> GetPackageAndClassName(const std::string & ClassPathName)
    {
        auto IndexIter = ClassPathName.find_last_of('/');
        if (IndexIter == ClassPathName.npos) {
            return std::make_pair(std::string(""), ClassPathName);
        }
        auto PackageName = ClassPathName.substr(0, IndexIter);
        for (auto & C : PackageName) {
            if (C == '/') {
                C = '.';
            }
        }
        auto ClassName = ClassPathName.substr(IndexIter + 1);
        return std::make_pair(std::move(PackageName), std::move(ClassName));
    }

    std::vector<std::string> GetInterfaceNames(const xClass & JavaClass)
    {
        std::vector<std::string> InterfaceNames;
        InterfaceNames.resize(JavaClass.InterfaceIndices.size());
        for (size_t Index = 0; Index < JavaClass.InterfaceIndices.size(); ++Index) {
            auto InterfaceIndex = JavaClass.InterfaceIndices[Index];
            InterfaceNames[Index] = GetClassName(*GetConstantItemClassPathName(JavaClass.ConstantPool, InterfaceIndex));
        }
        return InterfaceNames;
    }

    std::string GenerateClassTitle(const xClass & JavaClass)
    {
        std::vector<std::string> TitleStrings;

        // qualifierss:
        auto AccessFlags = JavaClass.AccessFlags;
        if (HasClassAccessFlag_Public(AccessFlags)) {
            TitleStrings.push_back("public");
        }
        if (HasClassAccessFlag_Abstract(AccessFlags)) {
            TitleStrings.push_back("abstract");
        }
        if (HasClassAccessFlag_Final(AccessFlags)) {
            TitleStrings.push_back("final");
        }

        // class type:
        if (HasClassAccessFlag_Enum(AccessFlags)) {
            TitleStrings.push_back("enum");
        }
        else if (HasClassAccessFlag_Interface(AccessFlags)) {
            if (HasClassAccessFlag_Annotation(AccessFlags)) {
                TitleStrings.push_back("@interface");
            } else {
                TitleStrings.push_back("interface");
            }
        }
        else {
            TitleStrings.push_back("class");
        }

        auto ThisClassName = *GetConstantItemClassPathName(JavaClass.ConstantPool, JavaClass.ThisClass);
        auto SuperClassName = *GetConstantItemClassPathName(JavaClass.ConstantPool, JavaClass.SuperClass);
        TitleStrings.push_back(GetClassName(ThisClassName));

        if (SuperClassName != JavaDefaultRootClassPathName) {
            TitleStrings.push_back("extends");
            TitleStrings.push_back(GetClassName(SuperClassName));
        }

        auto Interfaces = GetInterfaceNames(JavaClass);
        if (Interfaces.size()) {
            TitleStrings.push_back("implements");
            TitleStrings.push_back(Join(Interfaces.begin(), Interfaces.end(), ", "));
        }

        return Join(TitleStrings.begin(), TitleStrings.end(), ' ');
    }

    std::vector<std::string> GetImportNames(const xClass & JavaClass)
    {
        std::vector<std::string> ImportNames;
        for (size_t Index = 1 ; Index < JavaClass.ConstantPool.size(); ++Index) {
            auto & Item = JavaClass.ConstantPool[Index];
            if (Item.Tag != eConstantTag::Class) {
                continue;
            }
            if (Index == JavaClass.ThisClass) {
                continue;
            }
            auto ClassPathName = *GetConstantItemClassPathName(JavaClass.ConstantPool, Index);
            if (ClassPathName == JavaDefaultRootClassPathName) {
                continue;
            }
            auto FullClassName = GetFullClassName(ClassPathName);
            ImportNames.push_back(FullClassName);
        }
        return ImportNames;
    }

    std::string GenerateClassCode(const xClass & JavaClass)
    {
        auto ClassPathName = *GetConstantItemClassPathName(JavaClass.ConstantPool, JavaClass.ThisClass);
        auto ImportNames = GetImportNames(JavaClass);
        auto [PackageName, ClassName] = GetPackageAndClassName(ClassPathName);
        auto ClassTitle = GenerateClassTitle(JavaClass);

        std::stringstream ss;
        ss << "package " << PackageName << ';' << endl;
        ss << endl;

        for (const auto & ImportName : ImportNames) {
            ss << "import " << ImportName << ';' << endl;
        }
        ss << endl;

        ss << ClassTitle << " {" << endl;


        ss << endl << "}" << endl;

        return ss.str();
    }

}

