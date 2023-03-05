#include <jdc/jvm/JD_ClassInfo.hpp>
#include <xel/String.hpp>
#include <xel/Byte.hpp>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace xel;

namespace jdc
{
    const char * GetClassVersionString(uint16_t MajorVersion)
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

    const char * GetConstantTagString(const eConstantTag Tag)
    {
        switch (Tag) {
            case eConstantTag::Unspecified:
                return "<unspecified>";
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

    const char * GetFieldTypeString(const eFieldType Type)
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

    std::string EscapeString(const std::string & S)
    {
        std::ostringstream ss;
        for (auto c : S) {
            if (c == '\\') {
                ss << "\\\\";
                continue;
            }
            if (c == '"') {
                ss << "\\\"";
                continue;
            }
            ss << c;
        }
        return ss.str();
    }

    std::string EscapeStringQuoted(const std::string & S)
    {
        std::ostringstream ss;
        ss << '"';
        for (auto c : S) {
            if (c == '\\') {
                ss << "\\\\";
                continue;
            }
            if (c == '"') {
                ss << "\\\"";
                continue;
            }
            ss << c;
        }
        ss << '"';
        return ss.str();
    }

    const std::string & xClassInfo::GetConstantUtf8(size_t Index) const
    {
        auto & Item = ConstantPool[Index];
        assert(Item.Tag == eConstantTag::Utf8);
        return *Item.Info.Utf8.DataPtr;
    }

    const std::string & xClassInfo::GetConstantString(size_t Index) const
    {
        auto & Item = ConstantPool[Index];
        assert(Item.Tag == eConstantTag::String);
        return GetConstantUtf8(Item.Info.String.StringIndex);
    }

    const std::string & xClassInfo::GetConstantClassBinaryName(size_t Index) const
    {
        auto & Item = ConstantPool[Index];
        assert(Item.Tag == eConstantTag::Class);
        return GetConstantUtf8(Item.Info.Class.BinaryNameIndex);
    }

    const std::string xClassInfo::GetOutermostClassBinaryName() const
    {
        auto & ThisBinaryName = GetConstantClassBinaryName(ThisClass);
        auto Index = ThisBinaryName.find_last_of('$');
        if (Index == ThisBinaryName.npos) {
            return {};
        }
        return ThisBinaryName.substr(0, Index);
    }

    const std::string xClassInfo::GetConstantValueString(size_t Index) const
    {
        auto & Item = ConstantPool[Index];
        switch (Item.Tag) {
            case eConstantTag::Integer:
                return std::to_string(Item.Info.Integer.Value);
            case eConstantTag::Long:
                return std::to_string(Item.Info.Long.Value);
            case eConstantTag::Float:
                return std::to_string(Item.Info.Float.Value);
            case eConstantTag::Double:
                return std::to_string(Item.Info.Double.Value);
            case eConstantTag::Utf8:
                return *Item.Info.Utf8.DataPtr;
            case eConstantTag::String:
                return GetConstantUtf8(Item.Info.String.StringIndex);
            case eConstantTag::Class:
                return GetConstantUtf8(Item.Info.Class.BinaryNameIndex);
            default:
                break;
        }
        return {};
    }

    static const char * TranslateSimpleTypeBinaryName(const char C)
    {
        switch(C) {
            case 'B': {
                return "byte";
            }
            case 'C': {
                return "char";
            }
            case 'D': {
                return "double";
            }
            case 'F': {
                return "float";
            }
            case 'I': {
                return "int";
            }
            case 'J': {
                return "long";
            }
            case 'S': {
                return "short";
            }
            case 'Z': {
                return "boolean";
            }
            case 'V': {
                return "void";
            }
        }
        return nullptr;
    }

    const std::vector<std::string> xClassInfo::ExtractTypeBinaryNames(const std::string & Descriptor) const
    {
        X_DEBUG_PRINTF("xClassInfo::ExtractTypeBinaryNames: %s\n", Descriptor.c_str());
        size_t Index = 0;
        std::vector<std::string> BinaryNames;
        while(Index < Descriptor.size()) {
            auto C = Descriptor[Index];

            if (C == 'L') {
                const char * Start = &Descriptor[++Index];
                const char * End = Start + 1;
                while(*End != ';') {
                    ++End;
                }
                size_t Length = End - Start;
                Index += Length + 1;
                BinaryNames.push_back({ Start, Length });
                continue;
            }
            else if (C == '[') {
                size_t ArraySize = 1;
                while((C = Descriptor[++Index]) == '[') {
                    ++ArraySize;
                }
                std::string TypeName;
                if (C == 'L') {
                    const char * Start = &Descriptor[++Index];
                    const char * End = Start + 1;
                    while(*End != ';') {
                        ++End;
                    }
                    size_t Length = End - Start;
                    Index += Length + 1;
                    TypeName = std::string{ Start, Length };
                } else {
                    TypeName = TranslateSimpleTypeBinaryName(C);
                }
                while(ArraySize--) {
                    TypeName += "[]";
                }
                BinaryNames.push_back(std::move(TypeName));
                continue;
            }
            else if (C == '(' || C == ')') {
                ++Index;
                continue;
            }
            else { // SimpleTypeName
                BinaryNames.push_back(TranslateSimpleTypeBinaryName(C));
                ++Index;
                continue;
            }
        }
        return BinaryNames;
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
                Info.BinaryNameIndex = Reader.R2();
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
                if ((RemainSize -= 3) < 0) {
                    return false;
                }
                Info.ReferenceKind = Reader.R1();
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

    bool ExtractAttributeInfo(xStreamReader & Reader, ssize_t & RemainSize, xAttributeInfo & AttributeInfo)
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

    bool ExtractFieldInfo(xStreamReader & Reader, ssize_t & RemainSize, xFieldInfo & FieldInfo)
    {
        if ((RemainSize -= 8) < 0) {
            return false;
        }
        FieldInfo.AccessFlags = Reader.R2();
        FieldInfo.NameIndex = Reader.R2();
        FieldInfo.DescriptorIndex = Reader.R2();
        FieldInfo.Attributes.resize(Reader.R2());
        for (auto & AttributeInfo : FieldInfo.Attributes) {
            if (!ExtractAttributeInfo(Reader, RemainSize, AttributeInfo)) {
                return false;
            }
        }
        return true;
    }

    bool ExtractInnerClassAttribute(const std::vector<xel::ubyte> & Binary, std::vector<xInnerClassAttribute> & Output)
    {
        Output.clear();

        auto Reader = xStreamReader(Binary.data());
        auto RemainSize = ssize_t(Binary.size());
        if ((RemainSize -= 2) < 0) {
            return false;
        }
        size_t Total = Reader.R2();
        if ((RemainSize -= Total * 8) < 0) {
            return false;
        }
        for (size_t i = 0 ; i < Total; ++i) {
            xInnerClassAttribute ICA;
            ICA.InnerClassInfoIndex = Reader.R2();
            ICA.OuterClassInfoIndex = Reader.R2();
            ICA.InnerNameIndex = Reader.R2();
            ICA.InnerAccessFlags = Reader.R2();
            Output.push_back(ICA);
        }
        return true;
    }

    bool ExtractCodeAttribute(const std::vector<xel::ubyte> & Binary, xCodeAttribute & Output)
    {
        auto CA = xCodeAttribute();
        auto Reader = xStreamReader(Binary.data());
        ssize_t RemainSize = Binary.size();
        if ((RemainSize -= 8) < 0) {
            return false;
        }

        CA.MaxStack = Reader.R2();
        CA.MaxLocals = Reader.R2();
        size_t CodeBinarySize = Reader.R4();
        if ((RemainSize -= (CodeBinarySize + 2)) < 0) {
            return false;
        }
        CA.Binary.resize(CodeBinarySize);
        Reader.R(CA.Binary.data(), CodeBinarySize);

        size_t ExceptionTableLength = Reader.R2();
        if ((RemainSize -= ExceptionTableLength * 8 + 2) < 0) {
            return false;
        }
        for (size_t i = 0 ; i < ExceptionTableLength; ++i) {
            xExceptionTableItem Item;
            Item.StartPC = Reader.R2();
            Item.EndPC = Reader.R2();
            Item.HandlerPC = Reader.R2();
            Item.CatchType = Reader.R2();
            CA.ExceptionTable.push_back(std::move(Item));
        }

        size_t SubAttributeCount = Reader.R2();
        for (size_t i = 0 ; i < SubAttributeCount; ++i) {
            xAttributeInfo AttributeInfo;
            if (!ExtractAttributeInfo(Reader, RemainSize, AttributeInfo)) {
                return false;
            }
            CA.Attributes.push_back(std::move(AttributeInfo));
        }

        Output = std::move(CA);
        Output.Enabled = true;
        return true;
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
            if (!ExtractAttributeInfo(Reader, RemainSize, AttributeInfo)) {
                return false;
            }
        }
        return true;
    }

    xResult<xClassInfo> LoadClassInfoFromFile(const std::string & Filename)
    {
        xClassInfo JavaClass = {};

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
            if (!ExtractFieldInfo(Reader, RemainSize, FieldInfo)) {
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
            if (!ExtractAttributeInfo(Reader, RemainSize, AttributeInfo)) {
                return { JDR_DATA_SIZE_ERROR, "Read attribute info error @" X_STRINGIFY(__LINE__)};
            }
        }

        return { std::move(JavaClass) };
    }

}
