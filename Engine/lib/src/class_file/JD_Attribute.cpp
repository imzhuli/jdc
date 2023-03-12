#include <jdc/class_file/JD_Attribute.hpp>
#include <jdc/class_file/JD_ClassInfo.hpp>
#include <xel/Byte.hpp>

using namespace xel;

namespace jdc
{
    bool xAttributeBootstrapMethods::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());

        BootstrapMethods.resize(Reader.R2());
        for (auto & BootstrapMethod : BootstrapMethods) {
            BootstrapMethod.ReferenceIndex = Reader.R2();
            BootstrapMethod.ArgumentIndices.resize(Reader.R2());
            for (auto & Index : BootstrapMethod.ArgumentIndices) {
                Index = Reader.R2();
            }

            X_DEBUG_PRINTF("BootstrapMethod: Index=%u\n", (unsigned int)BootstrapMethod.ReferenceIndex);
        }
        return true;
    }

    bool xAttributeConstantValue::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        ValueIndex = Reader.R2();
        return true;
    }

    bool xAttributeCode::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());

        MaxStack = Reader.R2();
        MaxLocals = Reader.R2();

        uint32_t CodeSize = Reader.R4();
        CodeBinary.resize(CodeSize);
        Reader.R(CodeBinary.data(), CodeSize);

        uint16_t ExceptionTableLength = Reader.R2();
        ExceptionTables.resize(ExceptionTableLength);
        for (auto & ExceptionTable : ExceptionTables) {
            ExceptionTable.StartPC = Reader.R2();
            ExceptionTable.EndPC = Reader.R2();
            ExceptionTable.HandlePC = Reader.R2();
            ExceptionTable.CatchType = Reader.R2();
        }

        uint16_t SubAttributeCount = Reader.R2();
        SubAttributes.resize(SubAttributeCount);
        for (auto & Attribute : SubAttributes) {
            Attribute.NameIndex = Reader.R2();
            Attribute.Binary.resize(Reader.R4());
            Reader.R(Attribute.Binary.data(), Attribute.Binary.size());
        }

        return true;
    }

    bool xAttributeDeprecated::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        Deprecated = true;
        return true;
    }

    bool xAttributeEnclosingMethod::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        ClassIndex = Reader.R2();
        MethodIndex = Reader.R2();
        return true;
    }

    bool xAttributeExceptions::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        ExceptionIndexTable.resize(Reader.R2());
        for (auto & ExceptionIndex : ExceptionIndexTable) {
            ExceptionIndex = Reader.R2();
        }
        return true;
    }

    bool xAttributeInnerClasses::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        InnerClasses.resize(Reader.R2());
        for(auto & InnerClass : InnerClasses) {
            InnerClass.InnerClassInfoIndex = Reader.R2();
            InnerClass.OuterClassInfoIndex = Reader.R2();
            InnerClass.InnerNameIndex = Reader.R2();
            InnerClass.InnerAccessFlags = Reader.R2();
            // X_DEBUG_PRINTF("xInnerClasses: from:%s, Inner:%s, Outer:%s, InnerName:%s\n",
            //     ClassInfoPtr->GetConstantClassBinaryName(ClassInfoPtr->ThisClass).c_str(),
            //     ClassInfoPtr->GetConstantClassBinaryName(InnerClass.InnerClassInfoIndex).c_str(),
            //     ClassInfoPtr->GetConstantClassBinaryName(InnerClass.OuterClassInfoIndex).c_str(),
            //     ClassInfoPtr->GetConstantUtf8(InnerClass.InnerNameIndex).c_str()
            // );
        }
        return true;
    }

    bool xAttributeLineNumberTable::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        LineNumberTable.resize(Reader.R2());
        for (auto & LineNumber : LineNumberTable) {
            LineNumber.StartPC = Reader.R2();
            LineNumber.LineNumber = Reader.R2();
        }
        return true;
    }

    bool xAttributeLocalVariableTable::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        LocalVariableTable.resize(Reader.R2());

        for (auto & LocalVariable : LocalVariableTable) {
            LocalVariable.StartPC = Reader.R2();
            LocalVariable.Length = Reader.R2();
            LocalVariable.Name = ClassInfoPtr->GetConstantUtf8(Reader.R2());
            LocalVariable.Descriptor = ClassInfoPtr->GetConstantUtf8(Reader.R2());
            LocalVariable.Index = Reader.R2();

            X_DEBUG_PRINTF("LocalVariable: @%u, %s: %s\n", (unsigned int)LocalVariable.Index, LocalVariable.Name.c_str(), LocalVariable.Descriptor.c_str());
        }
        return true;
    }

    bool xAttributeLocalVariableTypeTable::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        LocalVariableTypeTable.resize(Reader.R2());

        for (auto & LocalVariableType : LocalVariableTypeTable) {
            LocalVariableType.StartPC = Reader.R2();
            LocalVariableType.Length = Reader.R2();
            LocalVariableType.Name = ClassInfoPtr->GetConstantUtf8(Reader.R2());
            LocalVariableType.Signature = ClassInfoPtr->GetConstantUtf8(Reader.R2());
            LocalVariableType.Index = Reader.R2();

            X_DEBUG_PRINTF("LocalVariableType: @%u, %s: %s\n", (unsigned int)LocalVariableType.Index, LocalVariableType.Name.c_str(), LocalVariableType.Signature.c_str());
        }
        return true;
    }

    bool xAttributeNestHost::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        HostClassIndex = Reader.R2();
        return true;
    }

    bool xAttributeNestMembers::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        NestClassIndices.resize(Reader.R2());

        for (auto & NetClassIndex : NestClassIndices) {
            NetClassIndex = Reader.R2();
        }
        return true;
    }

    bool xAttributeRuntimeAnnotations::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        Annotations.resize(Reader.R2());
        for (auto & Annotation : Annotations) {
            Annotation = LoadAnnotation(Reader);
        }
        return true;
    }

    bool xAttributeRuntimeParameterAnnotations::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        ParameterAnnotations.resize(Reader.R1());
        for (auto & Annotations : ParameterAnnotations) {
            Annotations.resize(Reader.R2());
            for (auto & Annotation : Annotations) {
                Annotation = LoadAnnotation(Reader);
                X_DEBUG_PRINTF("LoadAnnotation: TypeName:%s\n", ClassInfoPtr->GetConstantUtf8(Annotation->TypeNameIndex).c_str());
            }
        }
        return true;
    }

    bool xAttributeMethodParameters::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        Parameters.resize(Reader.R2());

        for (auto & Parameter : Parameters) {
            Parameter.Name = ClassInfoPtr->GetConstantUtf8(Reader.R2());
            Parameter.AccessFlags = Reader.R2();

            X_DEBUG_PRINTF("Parameter:%s: %x\n", Parameter.Name.c_str(), (unsigned int)Parameter.AccessFlags);
        }
        return true;
    }

    bool xAttributeSignature::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        uint16_t SignatureIndex = Reader.R2();
        Signature = ClassInfoPtr->GetConstantUtf8(SignatureIndex);
        return true;
    }

    bool xAttributeSourceFile::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        uint16_t SourceFileIndex = Reader.R2();
        Filename = ClassInfoPtr->GetConstantUtf8(SourceFileIndex);
        return true;
    }

    bool xAttributeSynthetic::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        Synthetic = true;
        return true;
    }

    /////////////////////////
    std::unique_ptr<xElementValue> LoadElementValue(xel::xStreamReader & Reader)
    {
        auto ElementValueUPtr = std::make_unique<xElementValue>();
        auto & ElementValue = *ElementValueUPtr;
        ElementValue.Tag = static_cast<eElementValueTag>(Reader.R1());

        if (ElementValue.Tag == eElementValueTag::Byte
        ||  ElementValue.Tag == eElementValueTag::Short
        ||  ElementValue.Tag == eElementValueTag::Int
        ||  ElementValue.Tag == eElementValueTag::Long
        ||  ElementValue.Tag == eElementValueTag::Char
        ||  ElementValue.Tag == eElementValueTag::Float
        ||  ElementValue.Tag == eElementValueTag::Double
        ||  ElementValue.Tag == eElementValueTag::Boolean
        ||  ElementValue.Tag == eElementValueTag::String) {
            ElementValue.ConstantValueIndex = Reader.R2();
            return ElementValueUPtr;
        }

        if (ElementValue.Tag == eElementValueTag::Class) {
            ElementValue.ClassIndex = Reader.R2();
            return ElementValueUPtr;
        }

        if (ElementValue.Tag == eElementValueTag::Enum) {
            ElementValue.EnumConstantValue.TypeNameIndex = Reader.R2();
            ElementValue.EnumConstantValue.NameIndex = Reader.R2();
            return ElementValueUPtr;
        }

        if (ElementValue.Tag == eElementValueTag::Annotation) {
            ElementValue.AnnotationUPtr = LoadAnnotation(Reader);
            return ElementValueUPtr;
        }

        if (ElementValue.Tag == eElementValueTag::Array) {
            uint16_t SubElementCount = Reader.R2();
            for (uint16_t i = 0 ; i < SubElementCount; ++i) {
                ElementValue.ArrayValues.push_back(LoadElementValue(Reader));
            }
            return ElementValueUPtr;
        }

        X_DEBUG_PRINTF("Invalid element value tag: %u", (unsigned int)ElementValue.Tag);
        return nullptr;
    }

    xElementValuePair LoadElementValuePair(xel::xStreamReader & Reader)
    {
        xElementValuePair Pair;
        Pair.ElementNameIndex = Reader.R2();
        Pair.ElementValueUPtr = LoadElementValue(Reader);
        return Pair;
    }

    std::unique_ptr<xAnnotation> LoadAnnotation(xel::xStreamReader & Reader)
    {
        auto AnnotationUPtr = std::make_unique<xAnnotation>();
        auto & Annotation = *AnnotationUPtr;

        Annotation.TypeNameIndex = Reader.R2();
        uint16_t ElementValuePairCount = Reader.R2();
        for (uint16_t i = 0 ; i < ElementValuePairCount ; ++i) {
            Annotation.ElementValuePairs.push_back(LoadElementValuePair(Reader));
        }
        return AnnotationUPtr;
    }

    /////////////////////////
    xAttributeMap LoadAttributeInfo(const std::vector<xAttributeInfo> & AttributeInfoList, const xClassInfo * ClassInfoPtr)
    {
        xAttributeMap Collection;
        for(auto & Info : AttributeInfoList) {
            auto & Name = ClassInfoPtr->GetConstantUtf8(Info.NameIndex);
            auto & Binary = Info.Binary;

            X_DEBUG_PRINTF("ExtractAttribute: %s\n", Name.c_str());

            if (Name == xAttributeNames::AnnotationDefault) {
                X_DEBUG_PRINTF("Unimplemented attribute reached: %s\n", Name.c_str());
                Fatal("Not implemented");
            }
            else if (Name == xAttributeNames::BootstrapMethods) {
                auto AttributeUPtr = std::make_unique<xAttributeBootstrapMethods>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::Code) {
                auto AttributeUPtr = std::make_unique<xAttributeCode>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::ConstantValue) {
                auto AttributeUPtr = std::make_unique<xAttributeConstantValue>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::Deprecated) {
                auto AttributeUPtr = std::make_unique<xAttributeDeprecated>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::EnclosingMethod) {
                auto AttributeUPtr = std::make_unique<xAttributeEnclosingMethod>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::Exceptions) {
                auto AttributeUPtr = std::make_unique<xAttributeExceptions>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::InnerClasses) {
                auto AttributeUPtr = std::make_unique<xAttributeInnerClasses>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::LocalVariableTable) {
                auto AttributeUPtr = std::make_unique<xAttributeLocalVariableTable>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::LocalVariableTypeTable) {
                auto AttributeUPtr = std::make_unique<xAttributeLocalVariableTypeTable>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::LineNumberTable) {
                auto AttributeUPtr = std::make_unique<xAttributeLineNumberTable>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::MethodParameters) {
                auto AttributeUPtr = std::make_unique<xAttributeMethodParameters>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::Module) {
                X_DEBUG_PRINTF("Ignore attribute %s\n", Name.c_str());
            }
            else if (Name == xAttributeNames::ModulePackages) {
                X_DEBUG_PRINTF("Ignore attribute %s\n", Name.c_str());
            }
            else if (Name == xAttributeNames::ModuleMainClass) {
                X_DEBUG_PRINTF("Ignore attribute %s\n", Name.c_str());
            }
            else if (Name == xAttributeNames::NestHost) {
                auto AttributeUPtr = std::make_unique<xAttributeNestHost>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::NestMembers) {
                auto AttributeUPtr = std::make_unique<xAttributeNestMembers>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::RuntimeInvisibleAnnotations) {
                auto AttributeUPtr = std::make_unique<xAttributeRuntimeAnnotations>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::RuntimeVisibleAnnotations) {
                auto AttributeUPtr = std::make_unique<xAttributeRuntimeAnnotations>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::RuntimeInvisibleParameterAnnotations) {
                auto AttributeUPtr = std::make_unique<xAttributeRuntimeParameterAnnotations>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::RuntimeVisibleParameterAnnotations) {
                auto AttributeUPtr = std::make_unique<xAttributeRuntimeParameterAnnotations>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::Signature) {
                auto AttributeUPtr = std::make_unique<xAttributeSignature>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::SourceFile) {
                auto AttributeUPtr = std::make_unique<xAttributeSourceFile>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else if (Name == xAttributeNames::Synthetic) {
                auto AttributeUPtr = std::make_unique<xAttributeSynthetic>();
                if (AttributeUPtr->Extract(Binary, ClassInfoPtr)) {
                    Collection.insert_or_assign(Name, std::move(AttributeUPtr));
                }
            }
            else {
                X_DEBUG_PRINTF("Unknonw attribute reached: %s\n", Name.c_str());
            }
        }
        return Collection;
    }

}
