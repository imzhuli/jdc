#include <jdc/JD_CodeGenerator.hpp>
#include <jdc/JD_ClassEx.hpp>
#include <jdc/JD_Util.hpp>
#include <xel/String.hpp>

using namespace std;
using namespace xel;

namespace jdc
{

    static const std::string JavaDefaultRootClassBinaryName = "java/lang/Object";

    std::vector<std::string> GetInterfaceNames(const xClassInfo & JavaClass)
    {
        std::vector<std::string> InterfaceNames;
        InterfaceNames.resize(JavaClass.InterfaceIndices.size());
        for (size_t Index = 0; Index < JavaClass.InterfaceIndices.size(); ++Index) {
            auto InterfaceIndex = JavaClass.InterfaceIndices[Index];
            InterfaceNames[Index] = GetClassName(*GetConstantItemClassBinaryName(JavaClass.ConstantPool, InterfaceIndex));
        }
        return InterfaceNames;
    }

    std::string GenerateClassTitle(const xClassInfo & JavaClass)
    {
        std::vector<std::string> TitleStrings;

        // qualifierss:
        auto AccessFlags = JavaClass.AccessFlags;
        if (AccessFlags & ACC_PUBLIC) {
            TitleStrings.push_back("public");
        }
        if (AccessFlags & ACC_ABSTRACT) {
            TitleStrings.push_back("abstract");
        }
        if (AccessFlags & ACC_FINAL) {
            TitleStrings.push_back("final");
        }

        // class type:
        if (AccessFlags & ACC_ENUM) {
            TitleStrings.push_back("enum");
        }
        else if (AccessFlags & ACC_INTERFACE) {
            if (AccessFlags & ACC_ANNOTATION) {
                TitleStrings.push_back("@interface");
            } else {
                TitleStrings.push_back("interface");
            }
        }
        else {
            TitleStrings.push_back("class");
        }

        auto ThisClassName = *GetConstantItemClassBinaryName(JavaClass.ConstantPool, JavaClass.ThisClass);
        auto SuperClassName = *GetConstantItemClassBinaryName(JavaClass.ConstantPool, JavaClass.SuperClass);
        TitleStrings.push_back(GetClassName(ThisClassName));

        if (SuperClassName != JavaDefaultRootClassBinaryName) {
            TitleStrings.push_back("extends");
            TitleStrings.push_back(GetClassName(SuperClassName));
        }

        auto Interfaces = GetInterfaceNames(JavaClass);
        if (Interfaces.size()) {
            TitleStrings.push_back("implements");
            TitleStrings.push_back(JoinStr(Interfaces.begin(), Interfaces.end(), ", "));
        }

        return JoinStr(TitleStrings.begin(), TitleStrings.end(), ' ');
    }

    std::vector<std::string> GetImportNames(const xClassInfo & JavaClass)
    {
        std::vector<std::string> ImportNames;
        return ImportNames;
    }

    std::string GenerateField(const std::vector<xConstantItemInfo> & ConstantPool, const xFieldInfo & FieldInfo)
    {
        return {};
    }

    std::string GenerateClassCode(const xClassInfo & JavaClass)
    {
        auto ClassBinaryName = *GetConstantItemClassBinaryName(JavaClass.ConstantPool, JavaClass.ThisClass);
        auto ImportNames = GetImportNames(JavaClass);
        auto [PackageName, ClassName] = GetPackageAndClassName(ClassBinaryName);
        auto ClassTitle = GenerateClassTitle(JavaClass);

        std::ostringstream ss;
        ss << "package " << PackageName << ';' << endl;
        ss << endl;

        for (const auto & ImportName : ImportNames) {
            ss << "import " << ImportName << ';' << endl;
        }
        ss << endl;

        ss << ClassTitle << " {" << endl;

        //////// fields:
        for (auto & FieldInfo : JavaClass.Fields) {
            auto FieldString = GenerateField(JavaClass.ConstantPool, FieldInfo);
            if (FieldString.size()) {
                ss << GenerateField(JavaClass.ConstantPool, FieldInfo) << endl;
            }
        }


        //// end of fields

        ss << endl << "}" << endl;

        return ss.str();
    }

}
