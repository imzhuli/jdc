#include <jdc/JD_ClassDump.hpp>
#include <jdc/JD_Util.hpp>
#include <jdc/JD_CodeGenerator.hpp>
#include <xel/String.hpp>
#include <xel/Byte.hpp>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace xel;

namespace jdc
{

    std::string DumpConstantItemString(const std::vector<xConstantItemInfo> & Items, size_t Index)
    {
        auto & Item = Items[Index];
        auto IndexString = "#" + std::to_string(Index) + " ";
        switch(Item.Tag)
        {
            case eConstantTag::Unspecified:
                return IndexString + "Unspecified";
            case eConstantTag::Utf8:
                return IndexString + "Utf8: " + *GetConstantItemUtf8(Item);
            case eConstantTag::Integer:
                return IndexString + "Integer: " + std::to_string(Item.Info.Integer.Value);
            case eConstantTag::Float:
                return IndexString + "Float: " + std::to_string(Item.Info.Float.Value);
            case eConstantTag::Long:
                return IndexString + "Long: " + std::to_string(Item.Info.Long.Value);
            case eConstantTag::Double:
                return IndexString + "Double: " + std::to_string(Item.Info.Double.Value);
            case eConstantTag::Class:
                return IndexString + "Class: @" + std::to_string(Item.Info.Class.PathNameIndex);
            case eConstantTag::String:
                return IndexString + "String: @" + std::to_string(Item.Info.String.StringIndex) + " ==> " + *GetConstantItemString(Items, Index);
            case eConstantTag::FieldRef:
                return IndexString + "FieldRef";
            case eConstantTag::MethodRef:
                return IndexString + "MethodRef";
            case eConstantTag::InterfaceMethodRef:
                return IndexString + "InterfaceMethodRef";
            case eConstantTag::NameAndType:
                return IndexString + "NameAndType";
            case eConstantTag::MethodHandle:
                return IndexString + "MethodHandle";
            case eConstantTag::MethodType:
                return IndexString + "MethodType";
            case eConstantTag::Dynamic:
                return IndexString + "Dynamic";
            case eConstantTag::InvokeDynamic:
                return IndexString + "InvokeDynamic";
            case eConstantTag::Module:
                return IndexString + "Module";
            case eConstantTag::Package:
                return IndexString + "Package";
            default:
                break;
        }
        return IndexString + "Unknown";
    }

    static std::string DumpClassAccessFlags(const xClass & JavaClass)
    {
        auto Flag = JavaClass.AccessFlags;
        std::vector<std::string> FlagStrings;
        if (HasClassAccessFlag_Super(Flag)) {
            FlagStrings.push_back("ACC_SUPER");
        }
        if (HasClassAccessFlag_Final(Flag)) {
            FlagStrings.push_back("ACC_FINAL");
        }
        if (HasClassAccessFlag_Public(Flag)) {
            FlagStrings.push_back("ACC_PUBLIC");
        }
        if (HasClassAccessFlag_Interface(Flag)) {
            FlagStrings.push_back("ACC_INTERFACE");
        }
        if (HasClassAccessFlag_Abstract(Flag)) {
            FlagStrings.push_back("ACC_ABSTRACT");
        }
        if (HasClassAccessFlag_Synthetic(Flag)) {
            FlagStrings.push_back("ACC_SYNTHETIC");
        }
        if (HasClassAccessFlag_Annotation(Flag)) {
            FlagStrings.push_back("ACC_ANNOTATION");
        }
        if (HasClassAccessFlag_Enum(Flag)) {
            FlagStrings.push_back("ACC_ENUM");
        }
        if (HasClassAccessFlag_Module(Flag)) {
            FlagStrings.push_back("ACC_MODULE");
        }
        return Join(FlagStrings.begin(), FlagStrings.end(), ' ');
    }

    std::string DumpStringFromClass(const xClass & JavaClass)
    {
        std::stringstream ss;
        ss << "ClassName " << *GetConstantItemClassPathName(JavaClass.ConstantPoolInfo, JavaClass.ThisClass) << endl;
        ss << "ClassName " << *GetConstantItemClassPathName(JavaClass.ConstantPoolInfo, JavaClass.SuperClass) << endl;
        ss << " - MagicCheck: " << YN(JavaClass.Magic == 0xCAFEBABE) << endl;
        ss << " - MajorVersion: " << JavaClass.MajorVersion << " ( " << ClassVersionString(JavaClass.MajorVersion) << " ) "<< endl;
        ss << " - MinorVersion: " << JavaClass.MinorVersion << endl;

        ss << " -- ConstantPoolSize: " << JavaClass.ConstantPoolInfo.size() << endl;
        for (size_t Index = 0; Index < JavaClass.ConstantPoolInfo.size(); ++Index) {
            ss << " --- " << DumpConstantItemString(JavaClass.ConstantPoolInfo, Index) << endl;
        }

        ss << " -- AccessFlags(0x" << std::hex << JavaClass.AccessFlags << std::dec << "): " << DumpClassAccessFlags(JavaClass) << endl;

        return ss.str();
    }

}
