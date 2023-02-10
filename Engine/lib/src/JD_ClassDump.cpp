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
                return IndexString + "<unspecified>";
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

    std::string DumpClassAccessFlags(const xClass & JavaClass)
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

    std::string DumpFieldAccessFlags(xAccessFlag Flags)
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

    std::string DumpMethodAccessFlags(xAccessFlag Flags)
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
        ss << HexShow(AttributeInfo.Binary.data(), AttributeInfo.Binary.size(), 4);
        return ss.str();
    }


    std::string DumpMethodDescriptor(const std::string & MethodName, const xMethodDescriptor & Descriptor)
    {
        std::ostringstream ss;
        ss << VariableTypeString(Descriptor.ReturnType);
        std::vector<std::string> ParamTypeStrings;

        for (size_t i = 0; i < Descriptor.ParameterTypes.size(); ++i) {
            auto & VType = Descriptor.ParameterTypes[i];
            if (VType.FieldType != eFieldType::Array) {
                ParamTypeStrings.push_back(VariableTypeString(VType));
                continue;
            }

            size_t ArraySize = 1;
            std::string ArrayTypeString;
            while(++i) {
                auto & TestVType = Descriptor.ParameterTypes[i];
                if (TestVType.FieldType != eFieldType::Array) {
                    ArrayTypeString = VariableTypeString(TestVType);
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

    std::string Dump(const std::vector<xConstantItemInfo> & ConstantPool, const xFieldEx & FieldEx)
    {
        std::ostringstream ss;
        ss << FieldEx.TypeString << ' ' <<  FieldEx.Name;
        if (FieldEx.InitValueString.size()) {
            ss << " = " << FieldEx.InitValueString;
        }
        return ss.str();
    }

    std::string Dump(const std::vector<xConstantItemInfo> & ConstantPool, const xMethodEx & MethodEx)
    {
        std::ostringstream ss;
        ss << MethodEx.TypeString << endl;
        if (MethodEx.CodeAttribute.Enabled) {
            auto & CA = MethodEx.CodeAttribute;
            ss << "MaxStack: " << CA.MaxStack << endl;
            ss << "MaxLocals: " << CA.MaxLocals << endl;
            ss << "ExceptionTableLength: " << CA.ExceptionTable.size() << endl;
            for (const auto & Item : CA.ExceptionTable) {
                ss << "  StartPC: " << Item.StartPC << endl;
                ss << "  EndPC: " << Item.EndPC << endl;
                ss << "  HandlerPC: " << Item.HandlerPC << endl;
                ss << "  CatchType: " << Item.CatchType << endl;
            }
            ss << "AttributeLength: " << CA.Attributes.size() << endl;
            for (const auto & Item : CA.Attributes) {
                ss << "  " << DumpAttribute(ConstantPool, Item) << endl;
            }
        }
        return ss.str();
    }

    std::string Dump(const std::vector<xConstantItemInfo> & ConstantPool, const xMethodInfo & MethodInfo)
    {
        std::ostringstream ss;
        for (auto & Attribute : MethodInfo.Attributes) {
            ss << DumpAttribute(ConstantPool, Attribute) << endl;
        }
        return ss.str();
    }

    std::string Dump(const xClass & JavaClass)
    {
        std::ostringstream ss;
        auto & ConstantPool = JavaClass.ConstantPool;

        // classex:
        auto ClassEx = Extend(JavaClass);
        ss << " - MagicCheck: " << YN(JavaClass.Magic == 0xCAFEBABE) << endl;
        ss << " - MajorVersion: " << JavaClass.MajorVersion << " ( " << ClassVersionString(JavaClass.MajorVersion) << " ) "<< endl;
        ss << " - MinorVersion: " << JavaClass.MinorVersion << endl;

        ss << " -- ConstantPoolSize: " << ConstantPool.size() << endl;
        for (size_t Index = 0; Index < ConstantPool.size(); ++Index) {
            ss << " ---- " << DumpConstantItemString(ConstantPool, Index) << endl;
        }
        ss << endl;

        ss << "SourceFile: " << ClassEx.SourceFile << endl;
        ss << "ClassName " << ClassEx.FullClassName << endl;
        ss << " -- SuperClassName " << ClassEx.FullSuperClassName << endl;
        ss << " -- AccessFlags(0x" << std::hex << JavaClass.AccessFlags << std::dec << "): " << DumpClassAccessFlags(JavaClass) << endl;

        ss << " -- Interfaces" << endl;
        for(auto & InterafceName : ClassEx.InterfaceNames) {
            ss << " ---- " << InterafceName << endl;
        }

        // dump fields:
        ss << " -- fields" << endl;
        for(auto & Field : JavaClass.Fields) {
            auto FieldEx = Extend(JavaClass, Field);
            ss << " ---- " << Dump(ConstantPool, FieldEx) << endl;
            for(auto & AttributeInfo : Field.Attributes) {
                ss << " ------ " << DumpAttribute(ConstantPool, AttributeInfo) << endl;
            }
        }

        // dump attributes:
        ss << " -- attributes" << endl;
        for(auto & AttributeInfo : JavaClass.Attributes) {
            ss << " ---- " << DumpAttribute(ConstantPool, AttributeInfo) << endl;
        }

        // dump methods:
        cout << endl;
        ss << "vvvvvvvvvv methods" << endl;
        for(auto & Method : JavaClass.Methods) {
            auto MethodEx = Extend(JavaClass, Method);
            ss << Dump(ConstantPool, MethodEx) << endl;
        }
        ss << "^^^^^^^^^^ end of methods" << endl;


        return ss.str();
    }

}
