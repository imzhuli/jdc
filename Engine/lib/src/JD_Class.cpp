#include <jdc/JD_Class.hpp>
#include <jdc/JD_ClassEx.hpp>
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

    const char * FieldTypeString(const eFieldType Type)
    {
        switch(Type) {
            case eFieldType::Void: {
                return "void";
            }
            case eFieldType::Byte: {
                return "byte";
            }
            case eFieldType::Short: {
                return "short";
            }
            case eFieldType::Integer: {
                return "int";
            }
            case eFieldType::Long: {
                return "long";
            }
            case eFieldType::Char: {
                return "char";
            }
            case eFieldType::Float: {
                return "float";
            }
            case eFieldType::Double: {
                return "double";
            }
            case eFieldType::Boolean: {
                return "boolean";
            }
            case eFieldType::Class: {
                return "Class";
            }
            case eFieldType::Array: {
                return "Array";
            }
            default: {
                break;
            }
        }
        return nullptr;
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

    const std::string * GetConstantItemClassPathName(const std::vector<xConstantItemInfo> & Items, size_t Index)
    {
        auto & Item = Items[Index];
        if (Item.Tag == eConstantTag::Class) {
            return GetConstantItemUtf8(Items, Item.Info.Class.PathNameIndex);
        }
        return nullptr;
    }

    const std::string ConstantValueString(const std::vector<xConstantItemInfo> & Items, size_t Index)
    {
        auto & Item = Items[Index];
        switch (Item.Tag) {
            case eConstantTag::Integer:
                return std::to_string(Item.Info.Integer.Value);
            case eConstantTag::Long:
                return std::to_string(Item.Info.Long.Value);
            case eConstantTag::Float:
                return std::to_string(Item.Info.Float.Value);
            case eConstantTag::Double:
                return std::to_string(Item.Info.Double.Value);
            case eConstantTag::String:
                return *GetConstantItemUtf8(Items, Item.Info.String.StringIndex);
            default:
                break;
        }
        return {};
    }

    const std::string ConstantFieldValueString(eFieldType FieldType, const std::vector<xConstantItemInfo> & Items, size_t Index)
    {
        auto & Item = Items[Index];
        switch (FieldType) {
            case eFieldType::Boolean: {
                assert(Item.Tag == eConstantTag::Integer);
                return Item.Info.Integer.Value ? "true" : "false";
            }
            case eFieldType::Byte:
            case eFieldType::Short:
            case eFieldType::Integer: {
                assert(Item.Tag == eConstantTag::Integer);
                return std::to_string(Item.Info.Integer.Value);
            }
            case eFieldType::Long: {
                assert(Item.Tag == eConstantTag::Long);
                return std::to_string(Item.Info.Long.Value);
            }
            case eFieldType::Float: {
                assert(Item.Tag == eConstantTag::Float);
                return std::to_string(Item.Info.Float.Value);
            }
            case eFieldType::Double: {
                assert(Item.Tag == eConstantTag::Double);
                return std::to_string(Item.Info.Double.Value);
            }
            case eFieldType::Class: {
                assert(Item.Tag == eConstantTag::String);
                return *GetConstantItemUtf8(Items, Item.Info.String.StringIndex);
            }
            default: {
                break;
            }
        }
        return {};
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
                Info.PathNameIndex = Reader.R2();
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

    std::string GetPackageName(const std::string & ClassPathName)
    {
        auto IndexIter = ClassPathName.rfind('/');
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
            if (C == '/' || C == '$') {
                C = '.';
            }
        }
        return Copy;
    }

    std::string GetClassName(const std::string & ClassPathName)
    {
        auto IndexIter = ClassPathName.rfind('$');
        if (IndexIter != ClassPathName.npos) {
            return ClassPathName.substr(IndexIter + 1);
        }

        IndexIter = ClassPathName.rfind('/');
        if (IndexIter != ClassPathName.npos) {
            return ClassPathName.substr(IndexIter + 1);
        }
        return ClassPathName;
    }

    std::pair<std::string, std::string> GetPackageAndClassName(const std::string & ClassPathName)
    {
        auto IndexIter = ClassPathName.rfind('/');
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
        for (auto & C : ClassName) {
            if (C == '$') {
                C = '.';
            }
        }
        return std::make_pair(std::move(PackageName), std::move(ClassName));
    }

    static bool LoadAttributeInfo(xStreamReader & Reader, ssize_t & RemainSize, xAttributeInfo & AttributeInfo)
    {
        if ((RemainSize -= 6) < 0) {
            return false;
        }
        AttributeInfo.NameIndex = Reader.R2();
        AttributeInfo.Binary.resize(Reader.R4());
        if ((RemainSize -= AttributeInfo.Binary.size()) < 0) {
            return false;
        }
        Reader.R(AttributeInfo.Binary.data(), AttributeInfo.Binary.size());
        return true;
    }

    static bool LoadFieldInfo(xStreamReader & Reader, ssize_t & RemainSize, xFieldInfo & FieldInfo)
    {
        if ((RemainSize -= 8) < 0) {
            return false;
        }
        FieldInfo.AccessFlags = Reader.R2();
        FieldInfo.NameIndex = Reader.R2();
        FieldInfo.DescriptorIndex = Reader.R2();
        FieldInfo.Attributes.resize(Reader.R2());
        for (auto & AttributeInfo : FieldInfo.Attributes) {
            if (!LoadAttributeInfo(Reader, RemainSize, AttributeInfo)) {
                return false;
            }
        }
        return true;
    }

    xVariableType ExtractVariableType(const std::string & Utf8, size_t & Index) {
        switch(Utf8[Index]) {
            case 'B': {
                ++Index;
                return { eFieldType::Byte };
            }
            case 'C': {
                ++Index;
                return { eFieldType::Char };
            }
            case 'D': {
                ++Index;
                return { eFieldType::Double };
            }
            case 'F': {
                ++Index;
                return { eFieldType::Float };
            }
            case 'I': {
                ++Index;
                return { eFieldType::Integer };
            }
            case 'J': {
                ++Index;
                return { eFieldType::Long };
            }
            case 'S': {
                ++Index;
                return { eFieldType::Short };
            }
            case 'Z': {
                ++Index;
                return { eFieldType::Boolean };
            }
            case '[': {
                ++Index;
                return { eFieldType::Array };
            }
            case 'L': {
                auto EndIndex = Utf8.find(';', Index);
                assert(EndIndex != Utf8.npos);
                assert(EndIndex != Index + 1);

                ++Index;
                xVariableType VType = { eFieldType::Class, Utf8.substr(Index, EndIndex - Index) };
                Index = EndIndex + 1;
                return VType;
            }
            case 'V': {
                ++Index;
                return { eFieldType::Void };
            }
            case '(': {
                ++Index;
                return { eFieldType::ParamStart };
            }
            case ')': {
                ++Index;
                return { eFieldType::ParamEnd };
            }
        }
        ++Index;
        return { eFieldType::Invalid };
    }

    std::string VariableTypeString(const xVariableType & VType)
    {
        if (VType.FieldType == eFieldType::Class) {
            return GetFullClassName(VType.ClassPathName);
        }
        return FieldTypeString(VType.FieldType);
    }

    std::string VariableTypeString(const std::string & Utf8)
    {
        size_t i = 0; // skip the first '('
        auto VType = ExtractVariableType(Utf8, i);
        if (VType.FieldType != eFieldType::Array) {
            return VariableTypeString(VType);
        }

        size_t ArraySize = 1;
        std::string ArrayTypeString;
        while (i < Utf8.size()) {
            auto TestVType = ExtractVariableType(Utf8, i);
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
        return ArrayTypeString;
    }

    xMethodDescriptor ExtractMethodDescriptor(const std::string & Utf8)
    {
        xMethodDescriptor Descriptor;
        size_t i = 1; // skip the first '('
        for (;i < Utf8.size();) {
            auto VType = ExtractVariableType(Utf8, i);
            if (VType.FieldType == eFieldType::ParamEnd) {
                break;
            }
            Descriptor.ParameterTypes.push_back(std::move(VType));
        }
        Descriptor.ReturnType = ExtractVariableType(Utf8, i);
        return Descriptor;
    }

    static bool LoadMethodInfo(xStreamReader & Reader, ssize_t & RemainSize, xMethodInfo & MethodInfo)
    {
        if ((RemainSize -= 8) < 0) {
            return false;
        }
        MethodInfo.AccessFlags = Reader.R2();
        MethodInfo.NameIndex = Reader.R2();
        MethodInfo.DescriptorIndex = Reader.R2();
        MethodInfo.Attributes.resize(Reader.R2());
        for (auto & AttributeInfo : MethodInfo.Attributes) {
            if (!LoadAttributeInfo(Reader, RemainSize, AttributeInfo)) {
                return false;
            }
        }
        return true;
    }

    xJDResult<xClass> LoadClassInfoFromFile(const std::string & Filename)
    {
        xClass JavaClass = {};

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

        // constant pool
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.ConstantPool.resize(Reader.R2());
        for (size_t Index = 1; Index < JavaClass.ConstantPool.size(); ++Index) { // !!! Note: Index starts from 1 to size - 1
            auto & Item = JavaClass.ConstantPool[Index];
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

        // this class
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.ThisClass = Reader.R2();

        // super class
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read class info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.SuperClass = Reader.R2();

        // interfaces
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read interface info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.InterfaceIndices.resize(Reader.R2());
        for (auto & Item : JavaClass.InterfaceIndices) {
            if ((RemainSize -= 2) < 0) {
                return { JDR_DATA_SIZE_ERROR, "Read interface info error @" X_STRINGIFY(__LINE__)};
            }
            Item = Reader.R2();
        }

        // fields:
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read field info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.Fields.resize(Reader.R2());
        for (auto & FieldInfo : JavaClass.Fields) {
            if (!LoadFieldInfo(Reader, RemainSize, FieldInfo)) {
                return { JDR_DATA_SIZE_ERROR, "Read field info error @" X_STRINGIFY(__LINE__)};
            }
        }

        // methods:
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read method info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.Methods.resize(Reader.R2());
        for (auto & MethodInfo : JavaClass.Methods) {
            if (!LoadMethodInfo(Reader, RemainSize, MethodInfo)) {
                return { JDR_DATA_SIZE_ERROR, "Read method info error @" X_STRINGIFY(__LINE__)};
            }
        }

        // attributes:
        if ((RemainSize -= 2) < 0) {
            return { JDR_DATA_SIZE_ERROR, "Read attribute info error @" X_STRINGIFY(__LINE__)};
        }
        JavaClass.Attributes.resize(Reader.R2());
        for (auto & AttributeInfo : JavaClass.Attributes) {
            if (!LoadAttributeInfo(Reader, RemainSize, AttributeInfo)) {
                return { JDR_DATA_SIZE_ERROR, "Read attribute info error @" X_STRINGIFY(__LINE__)};
            }
        }

        return { std::move(JavaClass) };
    }

}
