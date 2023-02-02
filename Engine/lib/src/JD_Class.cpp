#include <jdc/JD_Class.hpp>
#include <xel/String.hpp>
#include <xel/Byte.hpp>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace xel;

namespace xjd
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

    const char * ConstantTypeString(const eConstantTag Tag)
    {
        switch (Tag) {
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

    static bool LoadConstantInfo(xStreamReader & Reader, ssize_t & RemainSize, xConstantItemInfo & TagInfo)
    {
        if ((RemainSize -= 1) < 0) {
            return false;
        }
        TagInfo.Tag = eConstantTag(Reader.R1());
        switch(TagInfo.Tag) {
            case eConstantTag::Class: {
                auto & Info = TagInfo.ClassInfo;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.NameIndex = Reader.R2();
                return true;
            }
            case eConstantTag::FieldRef: {
                auto & Info = TagInfo.FieldRefInfo;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ClassIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                return true;
            }
            case eConstantTag::String: {
                auto & Info = TagInfo.StringInfo;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.StringIndex = Reader.R2();
                return true;
            }
            case eConstantTag::MethodRef: {
                auto & Info = TagInfo.MethodRefInfo;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ClassIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                return true;
            }
            case eConstantTag::InterfaceMethodRef: {
                auto & Info = TagInfo.InterfaceMethodRefInfo;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ClassIndex = Reader.R2();
                Info.NameAndTypeIndex = Reader.R2();
                return true;
            }
            case eConstantTag::Integer: {
                auto & Info = TagInfo.IntegerInfo;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.Value = Reader.R4();
                return true;
            }
            case eConstantTag::Float: {
                auto & Info = TagInfo.FloatInfo;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.Value = Reader.RF();
                return true;
            }
            case eConstantTag::Long: {
                auto & Info = TagInfo.LongInfo;
                if ((RemainSize -= 8) < 0) {
                    return false;
                }
                Info.Value = Reader.R8();
                return true;
            }
            case eConstantTag::Double: {
                auto & Info = TagInfo.DoubleInfo;
                if ((RemainSize -= 8) < 0) {
                    return false;
                }
                Info.Value = Reader.RD();
                return true;
            }
            case eConstantTag::NameAndType: {
                auto & Info = TagInfo.NameAndTypeInfo;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.NameIndex = Reader.R2();
                Info.DescriptorIndex = Reader.R2();
                return true;
            }
            case eConstantTag::Utf8: {
                auto & Info = TagInfo.Utf8Info;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                uint16_t Length = Reader.R2();
                if ((RemainSize -= Length) < 0) {
                    return false;
                }
                TagInfo.SetUtf8((const char *)Reader.Skip(Length), Length);
                return true;
            }
            case eConstantTag::MethodHandle: {
                auto & Info = TagInfo.MethodHandleInfo;
                if ((RemainSize -= 4) < 0) {
                    return false;
                }
                Info.ReferenceKind = Reader.R2();
                Info.ReferenceIndex = Reader.R2();
                return true;
            }
            case eConstantTag::MethodType: {
                auto & Info = TagInfo.MethodTypeInfo;
                if ((RemainSize -= 2) < 0) {
                    return false;
                }
                Info.DescriptorIndex = Reader.R2();
                return true;
            }
            default: {
                cerr << "Unknown Tag: value=" << (unsigned int)TagInfo.Tag << endl;
                break;
            }
        }
        return false;
    }

    xConstantItemInfo::xConstantItemInfo(const xConstantItemInfo &Other)
    {
        switch(Other.Tag) {
            case eConstantTag::Utf8: {
                auto & Source = Other.Utf8Info;
                SetUtf8(Source.DataPtr->data(), Source.DataPtr->size());
                Tag = Other.Tag;
                return;
            }
            default: {
                break;
            }
        }
        memcpy(this, &Other, sizeof(xConstantItemInfo));
    }

    xConstantItemInfo::~xConstantItemInfo()
    {
        Clear();
    }

    void xConstantItemInfo::SetUtf8(const char * DataPtr, size_t Length)
    {
        Clear();
        Tag = eConstantTag::Utf8;
        Utf8Info.DataPtr = new std::string(DataPtr, Length);
    }

    void xConstantItemInfo::Clear()
    {
        switch(Tag) {
            case eConstantTag::Utf8: {
                delete Utf8Info.DataPtr;
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
        for (auto & Item : JavaClass.ConstantPoolInfo) {
            if (!LoadConstantInfo(Reader, RemainSize, Item)) {
                return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
            }
        }

        return { JavaClass };
    }

    std::string DumpStringFromClass(const xClass & JavaClass)
    {
        std::stringstream ss;
        ss << "ClassName " << JavaClass.Name << endl;
        ss << " - MagicCheck: " << YN(JavaClass.Magic == 0xCAFEBABE) << endl;
        ss << " - MajorVersion: " << JavaClass.MajorVersion << " ( " << ClassVersionString(JavaClass.MajorVersion) << " ) "<< endl;
        ss << " - MinorVersion: " << JavaClass.MinorVersion << endl;

        ss << " -- ConstantPoolSize: " << JavaClass.ConstantPoolInfo.size() << endl;

        return ss.str();
    }

}
