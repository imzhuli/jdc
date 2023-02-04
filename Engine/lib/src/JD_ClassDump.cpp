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
        if (Flag & ACC_SUPER) {
            FlagStrings.push_back("ACC_SUPER");
        }
        if (Flag & ACC_FINAL) {
            FlagStrings.push_back("ACC_FINAL");
        }
        if (Flag & ACC_PUBLIC) {
            FlagStrings.push_back("ACC_PUBLIC");
        }
        if (Flag & ACC_INTERFACE) {
            FlagStrings.push_back("ACC_INTERFACE");
        }
        if (Flag & ACC_ABSTRACT) {
            FlagStrings.push_back("ACC_ABSTRACT");
        }
        if (Flag & ACC_SYNTHETIC) {
            FlagStrings.push_back("ACC_SYNTHETIC");
        }
        if (Flag & ACC_ANNOTATION) {
            FlagStrings.push_back("ACC_ANNOTATION");
        }
        if (Flag & ACC_ENUM) {
            FlagStrings.push_back("ACC_ENUM");
        }
        if (Flag & ACC_MODULE) {
            FlagStrings.push_back("ACC_MODULE");
        }
        return JoinStr(FlagStrings.begin(), FlagStrings.end(), ' ');
    }

    static std::string DumpFieldAccessFlags(xAccessFlag Flags)
    {
        std::vector<std::string> FlagStrings;
        if (Flags & ACC_PUBLIC) {
            FlagStrings.push_back("ACC_PUBLIC");
        }
        if (Flags & ACC_PRIVATE) {
            FlagStrings.push_back("ACC_PRIVATE");
        }
        if (Flags & ACC_PROTECTED) {
            FlagStrings.push_back("ACC_PROTECTED");
        }
        if (Flags & ACC_STATIC) {
            FlagStrings.push_back("ACC_STATIC");
        }
        if (Flags & ACC_FINAL) {
            FlagStrings.push_back("ACC_FINAL");
        }
        if (Flags & ACC_VOLATILE) {
            FlagStrings.push_back("ACC_VOLATILE");
        }
        if (Flags & ACC_TRANSIENT) {
            FlagStrings.push_back("ACC_TRANSIENT");
        }
        if (Flags & ACC_SYNTHETIC) {
            FlagStrings.push_back("ACC_SYNTHETIC");
        }
        if (Flags & ACC_ENUM) {
            FlagStrings.push_back("ACC_ENUM");
        }
        return JoinStr(FlagStrings.begin(), FlagStrings.end(), ' ');
    }

    static std::string DumpMethodAccessFlags(xAccessFlag Flags)
    {
        std::vector<std::string> FlagStrings;
        if (Flags & ACC_PUBLIC) {
            FlagStrings.push_back("ACC_PUBLIC");
        }
        if (Flags & ACC_PRIVATE) {
            FlagStrings.push_back("ACC_PRIVATE");
        }
        if (Flags & ACC_PROTECTED) {
            FlagStrings.push_back("ACC_PROTECTED");
        }
        if (Flags & ACC_STATIC) {
            FlagStrings.push_back("ACC_STATIC");
        }
        if (Flags & ACC_FINAL) {
            FlagStrings.push_back("ACC_FINAL");
        }
        if (Flags & ACC_SYNCHRONIZED) {
            FlagStrings.push_back("ACC_SYNCHRONIZED");
        }
        if (Flags & ACC_BRIDGE) {
            FlagStrings.push_back("ACC_BRIDGE");
        }
        if (Flags & ACC_VARARGS) {
            FlagStrings.push_back("ACC_VARARGS");
        }
        if (Flags & ACC_NATIVE) {
            FlagStrings.push_back("ACC_NATIVE");
        }
        if (Flags & ACC_ABSTRACT) {
            FlagStrings.push_back("ACC_ABSTRACT");
        }
        if (Flags & ACC_STRICT) {
            FlagStrings.push_back("ACC_STRICT");
        }
        if (Flags & ACC_SYNTHETIC) {
            FlagStrings.push_back("ACC_SYNTHETIC");
        }
        return JoinStr(FlagStrings.begin(), FlagStrings.end(), ' ');
    }


    std::string DumpAttribute(const std::vector<xConstantItemInfo> & ConstantPool, const xAttributeInfo & AttributeInfo)
    {
        std::ostringstream ss;
        ss << "Attribute: " << *GetConstantItemUtf8(ConstantPool, AttributeInfo.NameIndex) << ", size=" << AttributeInfo.Binary.size() << endl;
        return ss.str();
    }

    std::string DumpVariableType(const xVariableType & VType)
    {
        if (VType.Type == eFieldType::Class) {
            return GetFullClassName(VType.ClassPathName);
        }
        return FieldTypeString(VType.Type);
    }

    std::string DumpMethodDescriptor(const std::string & MethodName, const xMethodDescriptor & Descriptor)
    {
        std::ostringstream ss;
        ss << DumpVariableType(Descriptor.ReturnType);
        std::vector<std::string> ParamTypeStrings;

        for (size_t i = 0; i < Descriptor.ParameterTypes.size(); ++i) {
            auto & VType = Descriptor.ParameterTypes[i];
            if (VType.Type != eFieldType::Array) {
                ParamTypeStrings.push_back(DumpVariableType(VType));
                continue;
            }

            size_t ArraySize = 1;
            std::string ArrayTypeString;
            while(++i) {
                auto & TestVType = Descriptor.ParameterTypes[i];
                if (TestVType.Type != eFieldType::Array) {
                    ArrayTypeString = DumpVariableType(TestVType);
                    for (size_t ACounter = 0 ; ACounter < ArraySize; ++ACounter) {
                        ArrayTypeString += "[]";
                    }
                    break;
                } else {
                    ++ArraySize;
                }
            }
            ParamTypeStrings.push_back(ArrayTypeString);
        }
        ss << " " << MethodName << '(' << JoinStr(ParamTypeStrings.begin(), ParamTypeStrings.end(), ", ") << ')';
        return ss.str();
    }

    std::string DumpClass(const xClass & JavaClass)
    {
        std::ostringstream ss;

        ss << "ClassName " << *GetConstantItemClassPathName(JavaClass.ConstantPool, JavaClass.ThisClass) << endl;
        ss << "ClassName " << *GetConstantItemClassPathName(JavaClass.ConstantPool, JavaClass.SuperClass) << endl;
        ss << " - MagicCheck: " << YN(JavaClass.Magic == 0xCAFEBABE) << endl;
        ss << " - MajorVersion: " << JavaClass.MajorVersion << " ( " << ClassVersionString(JavaClass.MajorVersion) << " ) "<< endl;
        ss << " - MinorVersion: " << JavaClass.MinorVersion << endl;

        ss << " -- ConstantPoolSize: " << JavaClass.ConstantPool.size() << endl;
        for (size_t Index = 0; Index < JavaClass.ConstantPool.size(); ++Index) {
            ss << " ---- " << DumpConstantItemString(JavaClass.ConstantPool, Index) << endl;
        }

        ss << " -- AccessFlags(0x" << std::hex << JavaClass.AccessFlags << std::dec << "): " << DumpClassAccessFlags(JavaClass) << endl;

        ss << " -- Interfaces" << endl;
        for(auto InterfaceIndex : JavaClass.InterfaceIndices) {
            ss << " ---- " << *GetConstantItemClassPathName(JavaClass.ConstantPool, InterfaceIndex) << endl;
        }

        // dump fields:
        ss << " -- fields" << endl;
        for(auto & Field : JavaClass.Fields) {
            ss << " ---- " << *GetConstantItemUtf8(JavaClass.ConstantPool, Field.NameIndex) << " : " << DumpFieldAccessFlags(Field.AccessFlags) << endl;
            ss << " ------ " << "Descriptor: " << *GetConstantItemUtf8(JavaClass.ConstantPool, Field.DescriptorIndex) << endl;
            for(auto & AttributeInfo : Field.Attributes) {
                ss << " ------ " << DumpAttribute(JavaClass.ConstantPool, AttributeInfo);
                ss << HexShow(AttributeInfo.Binary.data(), AttributeInfo.Binary.size(), 8) << endl;
            }
        }

        // dump methods:
        ss << " -- methods" << endl;
        for(auto & Method : JavaClass.Methods) {
            auto & MethodName = *GetConstantItemUtf8(JavaClass.ConstantPool, Method.NameIndex);
            ss << " ---- " << MethodName << " : " << DumpMethodAccessFlags(Method.AccessFlags) << endl;

            auto DescriptorString =  *GetConstantItemUtf8(JavaClass.ConstantPool, Method.DescriptorIndex);
            auto Descriptor = ExtractMethodDescriptor(DescriptorString);
            ss << " ------ " << "Descriptor: " << DescriptorString << ": " << DumpMethodDescriptor(MethodName, Descriptor) << endl;
            for(auto & AttributeInfo : Method.Attributes) {
                ss << " ------ " << DumpAttribute(JavaClass.ConstantPool, AttributeInfo);
                ss << HexShow(AttributeInfo.Binary.data(), AttributeInfo.Binary.size(), 8) << endl;
            }
        }

        // dump attributes:
        ss << " -- attributes" << endl;
        for(auto & AttributeInfo : JavaClass.Attributes) {
            ss << " ---- " << DumpAttribute(JavaClass.ConstantPool, AttributeInfo);
            ss << HexShow(AttributeInfo.Binary.data(), AttributeInfo.Binary.size(), 6) << endl;
        }

        return ss.str();
    }

}
