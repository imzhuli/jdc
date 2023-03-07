#include <jdc/class_file/JD_Attribute.hpp>
#include <jdc/class_file/JD_ClassFile.hpp>
#include <xel/Byte.hpp>

using namespace xel;

namespace jdc
{

    bool xAttributeCode::Extract(const xAttributeBinary & AttributeBinary)
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

    bool xAttributeDeprecated::Extract(const xAttributeBinary & AttributeBinary)
    {
        Deprecated = true;
        return true;
    }

    bool xAttributeExceptions::Extract(const xAttributeBinary & AttributeBinary)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        ExceptionIndexTable.resize(Reader.R2());
        for (auto & ExceptionIndex : ExceptionIndexTable) {
            ExceptionIndex = Reader.R2();
        }
        return true;
    }

    bool xAttributeInnerClasses::Extract(const xAttributeBinary & AttributeBinary)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        InnerClassInfoIndex = Reader.R2();
        OuterClassInfoIndex = Reader.R2();
        InnerNameIndex = Reader.R2();
        InnerAccessFlags = Reader.R2();
        return true;
    }

    bool xAttributeLineNumberTable::Extract(const xAttributeBinary & AttributeBinary)
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

    bool xAttributeSourceFile::Extract(const xAttributeBinary & AttributeBinary, const xClassInfo * ClassInfoPtr)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        uint16_t SourceFileIndex = Reader.R2();
        SourceFile = ClassInfoPtr->GetConstantUtf8(SourceFileIndex);
        return true;
    }

    bool xAttributeSynthetic::Extract(const xAttributeBinary & AttributeBinary)
    {
        Synthetic = true;
        return true;
    }

}