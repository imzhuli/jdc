#include <jdc/JD_Class.hpp>
#include <jdc/JD_Util.hpp>
#include <xel/String.hpp>
#include <xel/Byte.hpp>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace xel;

namespace jdc
{
    const char * ClassVersionString(uint16_t MajorVersion)
    {
        if (MajorVersion <= 49) {
            return "Prior_to_1.5";
        }
        switch(MajorVersion) {
            case 50:
                return "Java_6";
            case 51:
                return "Java_7";
            case 52:
                return "Java_8";
            case 53:
                return "Java_9";
            case 54:
                return "Java_10";
            case 55:
                return "Java_11";
            case 56:
                return "Java_12";
            case 57:
                return "Java_13";
            case 58:
                return "Java_14";
            case 59:
                return "Java_15";
            case 60:
                return "Java_16";
            case 61:
                return "Java_17";
            case 62:
                return "Java_18";
            case 63:
                return "Java_19";
            default:
                break;
        }

        return "UnknownVersion";
    }

    const char * ConstantTagString(const eConstantTag Tag)
    {
        switch (Tag) {
            case eConstantTag::Unspecified:
                return "Unspecified";
            case eConstantTag::Utf8:
                return "Utf8";
            case eConstantTag::Integer:
                return "Integer";
            case eConstantTag::Float:
                return "Float";
            case eConstantTag::Long:
                return "Long";
            case eConstantTag::Double:
                return "Double";
            case eConstantTag::Class:
                return "Class";
            case eConstantTag::String:
                return "String";
            case eConstantTag::FieldRef:
                return "FieldRef";
            case eConstantTag::MethodRef:
                return "MethodRef";
            case eConstantTag::InterfaceMethodRef:
                return "InterfaceMethodRef";
            case eConstantTag::NameAndType:
                return "NameAndType";
            case eConstantTag::MethodHandle:
                return "MethodHandle";
            case eConstantTag::MethodType:
                return "MethodType";
            case eConstantTag::Dynamic:
                return "Dynamic";
            case eConstantTag::InvokeDynamic:
                return "InvokeDynamic";
            case eConstantTag::Module:
                return "Module";
            case eConstantTag::Package:
                return "Package";
            default:
                break;
        }
        return "Unknown";
    }

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
                return IndexString + "Class";
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

    const std::string * GetConstantItemUtf8(const xConstantItemInfo & Item)
    {
        if (Item.Tag == eConstantTag::Utf8) {
            return Item.Info.Utf8.DataPtr;
        }
        return nullptr;
    }

    const std::string * GetConstantItemUtf8(const std::vector<xConstantItemInfo> & Items, size_t Index)
    {
        auto & Item = Items[Index];
        return GetConstantItemUtf8(Item);
    }

    const std::string * GetConstantItemString(const std::vector<xConstantItemInfo> & Items, size_t Index)
    {
        auto & Item = Items[Index];
        if (Item.Tag == eConstantTag::String) {
            return GetConstantItemUtf8(Items, Item.Info.String.StringIndex);
        }
        return nullptr;
    }

    static bool LoadConstantInfo(xStreamReader & Reader, ssize_t & RemainSize, xConstantItemInfo & TagInfo)
    {
        if ((RemainSize -= 1) < 0) {
            return false;
        }
        auto Tag = eConstantTag(Reader.R1());
        switch(Tag) {
            case eConstantTag::Class: {
                auto & Info = TagInfo.Info.Class;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.NameIndex = Reader.R2();
                break;
            }
            case eConstantTag::FieldRef: {
                auto & Info = TagInfo.Info.FieldRef;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ClassIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                break;
            }
            case eConstantTag::String: {
                auto & Info = TagInfo.Info.String;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.StringIndex = Reader.R2();
                break;
            }
            case eConstantTag::MethodRef: {
                auto & Info = TagInfo.Info.MethodRef;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ClassIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                break;
            }
            case eConstantTag::InterfaceMethodRef: {
                auto & Info = TagInfo.Info.InterfaceMethodRef;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ClassIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                break;
            }
            case eConstantTag::Integer: {
                auto & Info = TagInfo.Info.Integer;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.Value = Reader.R4();
                break;
            }
            case eConstantTag::Float: {
                auto & Info = TagInfo.Info.Float;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.Value = Reader.RF();
                break;
            }
            case eConstantTag::Long: {
                auto & Info = TagInfo.Info.Long;
                if ((RemainSize -= 8) < 0) {
                    return false;
                }
                Info.Value = Reader.R8();
                break;
            }
            case eConstantTag::Double: {
                auto & Info = TagInfo.Info.Double;
                if ((RemainSize -= 8) < 0) {
                    return false;
                }
                Info.Value = Reader.RD();
                break;
            }
            case eConstantTag::NameAndType: {
                auto & Info = TagInfo.Info.NameAndType;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.NameIndex = Reader.R2();
                Info.DescriptorIndex = Reader.R2();
                break;
            }
            case eConstantTag::Utf8: {
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                uint16_t Length = Reader.R2();
                if ((RemainSize -= Length) < 0) {
                    return false;
                }
                TagInfo.SetUtf8((const char *)Reader.Skip(Length), Length);
                // no further operation is needed, just return
                return true;
            }
            case eConstantTag::MethodHandle: {
                auto & Info = TagInfo.Info.MethodHandle;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ReferenceKind = Reader.R2();
                Info.ReferenceIndex = Reader.R2();
                break;
            }
            case eConstantTag::MethodType: {
                auto & Info = TagInfo.Info.MethodType;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.DescriptorIndex = Reader.R2();
                break;
            }
            case eConstantTag::Dynamic: {
                auto & Info = TagInfo.Info.Dynamic;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.BootstrapMethodAttributeIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                break;
            }
            case eConstantTag::InvokeDynamic: {
                auto & Info = TagInfo.Info.InvokeDynamic;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.BootstrapMethodAttributeIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                break;
            }
            case eConstantTag::Module: {
                auto & Info = TagInfo.Info.Module;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.NameIndex = Reader.R2();
                break;
            }
            case eConstantTag::Package: {
                auto & Info = TagInfo.Info.Package;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.NameIndex = Reader.R2();
                break;
            }
            default: {
                cerr << "Unknown Tag: value=" << (unsigned int)TagInfo.Tag << endl;
                return false;
            }
        }

        TagInfo.Tag = Tag;
        return true;
    }

    xConstantItemInfo::xConstantItemInfo(const xConstantItemInfo &Other)
    {
        switch(Other.Tag) {
            case eConstantTag::Utf8: {
                auto & Source = Other.Info.Utf8;
                SetUtf8(Source.DataPtr->data(), Source.DataPtr->size());
                Tag = Other.Tag;
                return;
            }
            default: {
                break;
            }
        }
        Tag = Other.Tag;
        memcpy(&Info, &Other.Info, sizeof(Info));
    }

    xConstantItemInfo::xConstantItemInfo(xConstantItemInfo && Other)
    {
        Tag = Other.Tag;
        memcpy(&Info, &Other.Info, sizeof(Info));
        switch(Other.Tag) {
            case eConstantTag::Utf8: {
                Other.Tag = eConstantTag::Unspecified;
                Other.Info.Utf8.DataPtr = nullptr;
                return;
            }
            default: {
                break;
            }
        }
    }

    xConstantItemInfo::~xConstantItemInfo()
    {
        Clear();
    }

    void xConstantItemInfo::SetUtf8(const char * DataPtr, size_t Length)
    {
        Clear();
        Tag = eConstantTag::Utf8;
        Info.Utf8.DataPtr = new std::string(DataPtr, Length);
    }

    void xConstantItemInfo::Clear()
    {
        switch(Tag) {
            case eConstantTag::Utf8: {
                delete Info.Utf8.DataPtr;
                break;
            }
            default: {
                break;
            }
        }
        Tag = eConstantTag::Unspecified;
    }


    xJDResult<xClass> LoadClassInfoFromFile(const std::string & Filename)
    {
        xClass JavaClass = {};
        JavaClass.Name = std::filesystem::path(Filename).stem().string();

        auto FileDataOpt = FileToStr(Filename);
        if (!FileDataOpt()) {
            return { JDR_FILE_ERROR, "Failed to read file into memory block @" X_STRINGIFY(__LINE__) };
        }

        auto Reader = xStreamReader(FileDataOpt->data());
        auto RemainSize = (ssize_t)FileDataOpt->size();

        // read magic and version:
        if ((RemainSize -= 8) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.Magic        = Reader.R4();
        JavaClass.MinorVersion = Reader.R2();
        JavaClass.MajorVersion = Reader.R2();

        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.ConstantPoolInfo.resize(Reader.R2());
        for (size_t Index = 1; Index < JavaClass.ConstantPoolInfo.size(); ++Index) { // !!! Note: Index starts from 1 to size - 1
            auto & Item = JavaClass.ConstantPoolInfo[Index];
            if (!LoadConstantInfo(Reader, RemainSize, Item)) {
                return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
            }
            if (Item.Tag == eConstantTag::Long || Item.Tag == eConstantTag::Double) { // long & double types take two entry, an extra index increment is required
                ++Index;
            }
        }

        // read access flags:
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.AccessFlags = Reader.R2();


        return { std::move(JavaClass) };
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
        ss << "ClassName " << JavaClass.Name << endl;
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
