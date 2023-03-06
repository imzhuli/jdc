#include <jdc/class_file/JD_Attribute.hpp>
#include <xel/Byte.hpp>

using namespace xel;

namespace jdc
{

    bool xCodeAttribute::Extract(const xAttributeBinary & AttributeBinary)
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

    bool xInnerClassAttribute::Extract(const xAttributeBinary & AttributeBinary)
    {
        auto Reader = xStreamReader(AttributeBinary.data());
        InnerClassInfoIndex = Reader.R2();
        OuterClassInfoIndex = Reader.R2();
        InnerNameIndex = Reader.R2();
        InnerAccessFlags = Reader.R2();
        return true;
    }

}