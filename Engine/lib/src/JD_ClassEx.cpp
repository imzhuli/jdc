#include <jdc/JD_Class.hpp>
#include <jdc/JD_ClassEx.hpp>
#include <string>
#include <sstream>

using namespace xel;

namespace jdc
{
    uint16_t ExtractConstantValueAttribute(const std::vector<ubyte> & Binary)
    {
        assert(Binary.size() == 2);
        xStreamReader Reader(Binary.data());
        return Reader.R2();
    }

    uint16_t ExtractSourceAttribute(const std::vector<ubyte> & Binary)
    {
        assert(Binary.size() == 2);
        xStreamReader Reader(Binary.data());
        return Reader.R2();
    }

    xClassEx Extend(const xClass& JavaClass)
    {
        xClassEx Ex;
        auto & ConstantPool = JavaClass.ConstantPool;

        for (auto & AttributeInfo : JavaClass.Attributes) {
            auto & Name = *GetConstantItemUtf8(ConstantPool, AttributeInfo.NameIndex);
            if (Name == "SourceFile") {
                auto NameIndex = ExtractSourceAttribute(AttributeInfo.Binary);
                Ex.SourceFile = *GetConstantItemUtf8(JavaClass.ConstantPool, NameIndex);
            }
        }

        Ex.FullClassName = GetFullClassName(*GetConstantItemClassPathName(JavaClass.ConstantPool, JavaClass.ThisClass));
        Ex.FullSuperClassName = GetFullClassName(*GetConstantItemClassPathName(JavaClass.ConstantPool, JavaClass.SuperClass));

        for (auto InterfaceIndex : JavaClass.InterfaceIndices) {
            auto InterfaceName = *GetConstantItemClassPathName(JavaClass.ConstantPool, InterfaceIndex);
            Ex.InterfaceNames.push_back(InterfaceName);
        }
        return Ex;
    }

    xFieldEx Extend(const xClass& JavaClass, const xFieldInfo & FieldInfo)
    {
        xFieldEx Ex;
        auto & ConstantPool = JavaClass.ConstantPool;

        Ex.Name = *GetConstantItemUtf8(ConstantPool, FieldInfo.NameIndex);
        Ex.AccessFlags = FieldInfo.AccessFlags;

        // Type & Type String
        auto TypeDescriptor = *GetConstantItemUtf8(ConstantPool, FieldInfo.DescriptorIndex);
        size_t ExtractIndex = 0;
        Ex.Type = ExtractVariableType(TypeDescriptor, ExtractIndex);
        do {
            if (FieldInfo.AccessFlags & ACC_SYNTHETIC) {
                Ex.TypeString = {};
                break;
            }
            if (FieldInfo.AccessFlags & ACC_ENUM) {
                Ex.TypeString = {};
                break;
            }
            std::ostringstream ss;
            if (FieldInfo.AccessFlags & ACC_PUBLIC) {
                ss << "public ";
            }
            if (FieldInfo.AccessFlags & ACC_PRIVATE) {
                ss << "private ";
            }
            if (FieldInfo.AccessFlags & ACC_PROTECTED) {
                ss << "protected ";
            }
            if (FieldInfo.AccessFlags & ACC_STATIC) {
                ss << "static ";
            }
            if (FieldInfo.AccessFlags & ACC_FINAL) {
                ss << "final ";
            }
            if (FieldInfo.AccessFlags & ACC_VOLATILE) {
                ss << "volatile ";
            }
            if (FieldInfo.AccessFlags & ACC_TRANSIENT) {
                ss << "transient ";
            }
            ss << VariableTypeString(TypeDescriptor);
            Ex.TypeString = ss.str();
        } while(false);

        for (auto & Attribute : FieldInfo.Attributes) {
            auto AttributeName = *GetConstantItemUtf8(ConstantPool, Attribute.NameIndex);

            if (AttributeName == "ConstantValue") {
                auto ValueIndex = ExtractConstantValueAttribute(Attribute.Binary);
                Ex.InitValueString = ConstantFieldValueString(Ex.Type.FieldType, ConstantPool, ValueIndex);
            }
        }

        return Ex;
    }

};

