#include <jdc/jvm/JD_Class.hpp>
#include <jdc/JD_ClassEx.hpp>
#include <xel/String.hpp>

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

    xClassEx Extend(const xClassInfo& JavaClass)
    {
        xClassEx Ex;
        auto & ConstantPool = JavaClass.ConstantPool;

        for (auto & AttributeInfo : JavaClass.Attributes) {
            auto & Name = *GetConstantItemUtf8(ConstantPool, AttributeInfo.NameIndex);
            if (Name == "SourceFile") {
                auto NameIndex = ExtractSourceAttribute(AttributeInfo.Binary);
                Ex.SourceFile = *GetConstantItemUtf8(JavaClass.ConstantPool, NameIndex);
                continue;
            }
            if (Name == "InnerClasses") {
                if (!ExtractInnerClassAttribute(AttributeInfo.Binary, Ex.InnerClasses)) {
                    assert("Failed to ExtractInnerClassAttribute @" X_STRINGIFY(__LINE__));
                }
                continue;
            }
        }

        Ex.FullClassName = GetFullClassName(*GetConstantItemClassBinaryName(JavaClass.ConstantPool, JavaClass.ThisClass));
        Ex.FullSuperClassName = GetFullClassName(*GetConstantItemClassBinaryName(JavaClass.ConstantPool, JavaClass.SuperClass));

        for (auto InterfaceIndex : JavaClass.InterfaceIndices) {
            auto InterfaceName = *GetConstantItemClassBinaryName(JavaClass.ConstantPool, InterfaceIndex);
            Ex.InterfaceNames.push_back(InterfaceName);
        }
        return Ex;
    }

    xFieldEx Extend(const xClassInfo& JavaClass, const xFieldInfo & FieldInfo)
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

    xMethodEx Extend(const xClassInfo& JavaClass, const xMethodInfo & MethodInfo)
    {
        xMethodEx Ex;
        auto & ConstantPool = JavaClass.ConstantPool;

        Ex.ClassName = GetFullClassName(*GetConstantItemClassBinaryName(JavaClass.ConstantPool, JavaClass.ThisClass));
        Ex.Name = *GetConstantItemUtf8(ConstantPool, MethodInfo.NameIndex);

        do {
            std::ostringstream ss;
            Ex.AccessFlags = MethodInfo.AccessFlags;
            if (MethodInfo.AccessFlags & ACC_SYNTHETIC) {
                ss << "<synthetic> ";
            }
            if (MethodInfo.AccessFlags & ACC_BRIDGE) {
                ss << "<bridged> ";
            }
            if (MethodInfo.AccessFlags & ACC_PUBLIC) {
                ss << "public ";
            }
            if (MethodInfo.AccessFlags & ACC_PRIVATE) {
                ss << "private ";
            }
            if (MethodInfo.AccessFlags & ACC_PROTECTED) {
                ss << "protected ";
            }
            if (MethodInfo.AccessFlags & ACC_STATIC) {
                ss << "static ";
            }
            if (MethodInfo.AccessFlags & ACC_FINAL) {
                ss << "final ";
            }
            if (MethodInfo.AccessFlags & ACC_SYNCHRONIZED) {
                ss << "synchronized ";
            }
            if (MethodInfo.AccessFlags & ACC_VARARGS) {
                // do nothing here
            }
            if (MethodInfo.AccessFlags & ACC_NATIVE) {
                // do nothing here
            }
            if (MethodInfo.AccessFlags & ACC_ABSTRACT) {
                ss << "abstract ";
            }
            if (MethodInfo.AccessFlags & ACC_STRICT) {
                ss << "strictfp ";
            }

            auto DescriptorString =  *GetConstantItemUtf8(ConstantPool, MethodInfo.DescriptorIndex);
            auto Descriptor = ExtractMethodDescriptor(DescriptorString);
            ss << VariableTypeString(Descriptor.ReturnType);
            std::vector<std::string> ParamTypeStrings;

            size_t ArgumentSize = 0;
            for (size_t i = 0; i < Descriptor.ParameterTypes.size(); ++i) {
                auto & VType = Descriptor.ParameterTypes[i];
                if (VType.FieldType != eFieldType::Array) {
                    auto TypeNameString = VariableTypeString(VType);
                    Ex.ArgumentTypeStrings.push_back(TypeNameString);
                    ParamTypeStrings.push_back(TypeNameString + ' ' + MakeArgumentName(ArgumentSize++));
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
                Ex.ArgumentTypeStrings.push_back(ArrayTypeString);
                ParamTypeStrings.push_back(ArrayTypeString + ' ' + MakeArgumentName(ArgumentSize++));
            }
            ss << " " << Ex.Name << '(' << JoinStr(ParamTypeStrings.begin(), ParamTypeStrings.end(), ", ") << ')';
            Ex.TypeString = ss.str();
        } while(false);

        // If the method is either native or abstract,
        // and is not a class or interface initialization method, then its method_info structure must not have a Code attribute in its attributes table.
        // Otherwise, its method_info structure must have exactly one Code attribute in its attributes table.
        for (auto & Attribute : MethodInfo.Attributes) {
            const auto & Name = *GetConstantItemUtf8(ConstantPool, Attribute.NameIndex);
            if (Name == "Code") {
                if (!ExtractCodeAttribute(Attribute.Binary, Ex.CodeAttribute)) {
                    assert(!"Failed to ExtractCodeAttribute @" X_STRINGIFY(__LINE__));
                }
                continue;
            }
        }

        return Ex;
    }

};

