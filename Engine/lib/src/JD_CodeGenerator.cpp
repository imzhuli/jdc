#include <jdc/JD_CodeGenerator.hpp>
#include <jdc/JD_Util.hpp>

namespace jdc
{

    static const char * JavaDefaultRootClassName = "java/lang/Object";

    std::string GetClassFullName(const std::string & ClassNameBySlash)
    {
        std::string Copy = ClassNameBySlash;
        for (auto & c : Copy) {
            if (c == '/') {
                c = '.';
            }
        }
        return Copy;
    }

    std::string GetClassStemName(const std::string & ClassNameBySlash)
    {
        auto IndexIter = ClassNameBySlash.find_last_of('/');
        if (IndexIter == ClassNameBySlash.npos) {
            return ClassNameBySlash;
        }
        return ClassNameBySlash.substr(IndexIter + 1);
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

        auto ThisClassName = *GetConstantItemClassName(JavaClass.ConstantPoolInfo, JavaClass.ThisClass);
        auto SuperClassName = *GetConstantItemClassName(JavaClass.ConstantPoolInfo, JavaClass.SuperClass);
        TitleStrings.push_back(GetClassStemName(ThisClassName));

        if (SuperClassName != JavaDefaultRootClassName) {
            TitleStrings.push_back("extends");
            TitleStrings.push_back(GetClassStemName(SuperClassName));
        }

        return Join(TitleStrings.begin(), TitleStrings.end(), ' ');
    }

}

