#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <xel/String.hpp>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace xel;

namespace jdc
{

    using namespace std::literals::string_literals;

    #define BLOCK_TYPE_TO_STRING(x) case (xJavaBlock::x): return #x##s
    std::string ToString(const xJavaBlock::eType Type)
    {
        switch(Type) {
            BLOCK_TYPE_TO_STRING(TYPE_DELETED);
            BLOCK_TYPE_TO_STRING(TYPE_START);
            BLOCK_TYPE_TO_STRING(TYPE_END);
            BLOCK_TYPE_TO_STRING(TYPE_STATEMENTS);
            BLOCK_TYPE_TO_STRING(TYPE_THROW);
            BLOCK_TYPE_TO_STRING(TYPE_RETURN);
            BLOCK_TYPE_TO_STRING(TYPE_RETURN_VALUE);
            BLOCK_TYPE_TO_STRING(TYPE_SWITCH_DECLARATION);
            BLOCK_TYPE_TO_STRING(TYPE_SWITCH);
            BLOCK_TYPE_TO_STRING(TYPE_SWITCH_BREAK);
            BLOCK_TYPE_TO_STRING(TYPE_TRY_DECLARATION);
            BLOCK_TYPE_TO_STRING(TYPE_TRY);
            BLOCK_TYPE_TO_STRING(TYPE_TRY_JSR);
            BLOCK_TYPE_TO_STRING(TYPE_TRY_ECLIPSE);
            BLOCK_TYPE_TO_STRING(TYPE_JSR);
            BLOCK_TYPE_TO_STRING(TYPE_RET);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITIONAL_BRANCH);
            BLOCK_TYPE_TO_STRING(TYPE_IF);
            BLOCK_TYPE_TO_STRING(TYPE_IF_ELSE);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION_OR);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION_AND);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION_TERNARY_OPERATOR);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP_START);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP_CONTINUE);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP_END);
            BLOCK_TYPE_TO_STRING(TYPE_GOTO);
            BLOCK_TYPE_TO_STRING(TYPE_INFINITE_GOTO);
            BLOCK_TYPE_TO_STRING(TYPE_GOTO_IN_TERNARY_OPERATOR);
            BLOCK_TYPE_TO_STRING(TYPE_TERNARY_OPERATOR);
            BLOCK_TYPE_TO_STRING(TYPE_JUMP);
        default:
            break;
        }
        return "TYPE_INVALID"s;
    };

    std::string ToString(const xJavaBlock * BlockPtr)
    {
        auto OS = std::ostringstream();
        OS << "BasicBlock{index=" << BlockPtr->Index
            << ", from=" << BlockPtr->FromOffset
            << ", to=" << BlockPtr->ToOffset
            << ", type=" << (ToString(BlockPtr->Type).c_str() + 5)
            << ", inverseCondition=" << TF(BlockPtr->MustInverseCondition);

        std::vector<size_t> IndexList;
        for (auto & PredecessorPtr : BlockPtr->Predecessors) {
            IndexList.push_back(PredecessorPtr->Index);
        }
        std::sort(IndexList.begin(), IndexList.end());

        if (IndexList.size()) {
            bool First = true;
            OS << ", predecessors=[";
            for (auto Index : IndexList) {
                if (Steal(First, false)) {
                    OS << Index;
                } else {
                    OS << ", " << Index;
                }
            }
            OS << "]";
        }

        OS << "}";
        return OS.str();
    }

    xJavaBlock::xJavaBlock(xJavaControlFlowGraph * CFGPtr, eType Type, size_t FromOffset, size_t ToOffset)
    : _JavaControlFlowGraphPtr(CFGPtr), Type(Type), FromOffset(FromOffset), ToOffset(ToOffset)
    {
        auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(GetMethod()->Converted.AttributeMap, "Code");
        assert(CodeAttributePtr);
        _CodeBinaryPtr = &CodeAttributePtr->CodeBinary;
    }

    const xJavaClass * xJavaBlock::GetClass() const
    {
        return _JavaControlFlowGraphPtr->GetClass();
    }

    const xJavaMethod * xJavaBlock::GetMethod() const
    {
        return _JavaControlFlowGraphPtr->GetMethod();
    }

    xOpCode xJavaBlock::GetNextOpCode(size_t MaxOffset) const
    {
        auto & CodeBinary = *GetCode();
        size_t Offset   = FromOffset;
        size_t ToOffset = this->ToOffset;

        if (ToOffset > MaxOffset) {
            ToOffset = MaxOffset;
        }

        auto Reader = xel::xStreamReader(CodeBinary.data());
        for (; Offset < ToOffset; Offset++) {
            xOpCode OpCode = CodeBinary[Offset];
            switch (OpCode) {
                case 16: case 18: // BIPUSH, LDC
                case 21: case 22: case 23: case 24: case 25: // ILOAD, LLOAD, FLOAD, DLOAD, ALOAD
                case 54: case 55: case 56: case 57: case 58: // ISTORE, LSTORE, FSTORE, DSTORE, ASTORE
                case 169: // RET
                case 188: // NEWARRAY
                    Offset++;
                    break;
                case 17: // SIPUSH
                case 19: case 20: // LDC_W, LDC2_W
                case 132: // IINC
                case 178: // GETSTATIC
                case 179: // PUTSTATIC
                case 187: // NEW
                case 180: // GETFIELD
                case 181: // PUTFIELD
                case 182: case 183: // INVOKEVIRTUAL, INVOKESPECIAL
                case 184: // INVOKESTATIC
                case 189: // ANEWARRAY
                case 192: // CHECKCAST
                case 193: // INSTANCEOF
                    Offset += 2;
                    break;
                case 153: case 154: case 155: case 156: case 157: case 158: // IFEQ, IFNE, IFLT, IFGE, IFGT, IFLE
                case 159: case 160: case 161: case 162: case 163: case 164: case 165: case 166: // IF_ICMPEQ, IF_ICMPNE, IF_ICMPLT, IF_ICMPGE, IF_ICMPGT, IF_ICMPLE, IF_ACMPEQ, IF_ACMPNE
                case 167: // GOTO
                case 198: case 199: { // IFNULL, IFNONNULL
                    Reader.Offset(Offset);
                    auto DeltaOffset = (int16_t)Reader.R2();
                    Offset += 2;
                    if (DeltaOffset > 0) {
                        Offset += DeltaOffset - 2 - 1;
                    }
                    break;
                }
                case 200: { // GOTO_W
                    Reader.Offset(Offset);
                    auto DeltaOffset = (int32_t)Reader.R4();
                    Offset += 4;
                    if (DeltaOffset > 0) {
                        Offset += DeltaOffset - 4 - 1;
                    }
                    break;
                }
                case 168: // JSR
                    Offset += 2;
                    break;
                case 197: // MULTIANEWARRAY
                    Offset += 3;
                    break;
                case 185: // INVOKEINTERFACE
                case 186: // INVOKEDYNAMIC
                    Offset += 4;
                    break;
                case 201: // JSR_W
                    Offset += 4;
                    break;
                case 170: { // TABLESWITCH
                    Offset = (Offset + 4) & 0xFFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    Reader.Offset(Offset);
                    int32_t Low  = Reader.R4();
                    int32_t High = Reader.R4();
                    Offset += 8;
                    Offset += (4 * (High - Low + 1)) - 1;
                    break;
                }
                case 171: { // LOOKUPSWITCH
                    Offset = (Offset + 4) & 0xFFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    Reader.Offset(Offset);
                    int32_t Count = Reader.R4();
                    Offset += 4;
                    Offset += (8 * Count) - 1;
                    break;
                }
                case 196: // WIDE
                    OpCode = CodeBinary[++Offset];
                    if (OpCode == 132) { // IINC
                        Offset += 4;
                    } else {
                        Offset += 2;
                    }
                    break;
                default:
                    break;
            }
        }

        if (Offset <= MaxOffset) {
            return CodeBinary[Offset];
        }
        return 0;
    }

    xOpCode xJavaBlock::GetLastOpCode() const
    {
        auto & CodeBinary = *GetCode();
        size_t Offset = this->FromOffset;
        size_t ToOffset = this->ToOffset;
        if (Offset >= ToOffset) {
            return 0;
        }
        size_t LastOffset = ToOffset;
        auto Reader = xel::xStreamReader(CodeBinary.data());
        for (; Offset < ToOffset; Offset++) {
            xOpCode OpCode = CodeBinary[Offset];
            LastOffset = Offset;
            switch (OpCode) {
                case 16: case 18: // BIPUSH, LDC
                case 21: case 22: case 23: case 24: case 25: // ILOAD, LLOAD, FLOAD, DLOAD, ALOAD
                case 54: case 55: case 56: case 57: case 58: // ISTORE, LSTORE, FSTORE, DSTORE, ASTORE
                case 169: // RET
                case 188: // NEWARRAY
                    Offset++;
                    break;
                case 17: // SIPUSH
                case 19: case 20: // LDC_W, LDC2_W
                case 132: // IINC
                case 178: // GETSTATIC
                case 179: // PUTSTATIC
                case 187: // NEW
                case 180: // GETFIELD
                case 181: // PUTFIELD
                case 182: case 183: // INVOKEVIRTUAL, INVOKESPECIAL
                case 184: // INVOKESTATIC
                case 189: // ANEWARRAY
                case 192: // CHECKCAST
                case 193: // INSTANCEOF
                    Offset += 2;
                    break;
                case 153: case 154: case 155: case 156: case 157: case 158: // IFEQ, IFNE, IFLT, IFGE, IFGT, IFLE
                case 159: case 160: case 161: case 162: case 163: case 164: case 165: case 166: // IF_ICMPEQ, IF_ICMPNE, IF_ICMPLT, IF_ICMPGE, IF_ICMPGT, IF_ICMPLE, IF_ACMPEQ, IF_ACMPNE
                case 167: // GOTO
                case 198: case 199: { // IFNULL, IFNONNULL
                    Reader.Offset(Offset);
                    auto DeltaOffset = (int16_t)Reader.R2();
                    Offset += 2;
                    if (DeltaOffset > 0) {
                        Offset += DeltaOffset - 2 - 1;
                    }
                    break;
                }
                case 200: { // GOTO_W
                    Reader.Offset(Offset);
                    auto DeltaOffset = (int32_t)Reader.R4();
                    Offset += 4;
                    if (DeltaOffset > 0) {
                        Offset += DeltaOffset - 4 - 1;
                    }
                    break;
                }
                case 168: // JSR
                    Offset += 2;
                    break;
                case 197: // MULTIANEWARRAY
                    Offset += 3;
                    break;
                case 185: // INVOKEINTERFACE
                case 186: // INVOKEDYNAMIC
                    Offset += 4;
                    break;
                case 201: // JSR_W
                    Offset += 4;
                    break;
                case 170: { // TABLESWITCH
                    Offset = (Offset + 4) & 0xFFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    Reader.Offset(Offset);
                    auto Low = (int32_t)Reader.R4();
                    auto High = (int32_t)Reader.R4();
                    Offset += 8;
                    Offset += (4 * (High - Low + 1)) - 1;
                    break;
                }
                case 171: { // LOOKUPSWITCH
                    Offset = (Offset + 4) & 0xFFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    Reader.Offset(Offset);
                    auto Count = (int32_t)Reader.R4();
                    Offset += 4;
                    Offset += (8 * Count) - 1;
                    break;
                }
                case 196: // WIDE
                    OpCode = CodeBinary[++Offset];
                    if (OpCode == 132) { // IINC
                        Offset += 4;
                    } else {
                        OpCode += 2;
                    }
                    break;
                default:
                    break;
            }
        }
        return CodeBinary[LastOffset];
    }

    xel::ssize_t xJavaBlock::EvalStackDepth() const
    {
        ssize_t Depth = 0;
        auto & ClassInfo = GetClass()->ClassInfo;
        auto & CodeBinary = *GetCode();
        for (size_t Offset = this->FromOffset, ToOffset = this->ToOffset; Offset < ToOffset; ++Offset) {
            xOpCode OpCode = CodeBinary[Offset];
            switch(OpCode) {
                case 1: // ACONST_NULL
                case 2: case 3: case 4: case 5: case 6: case 7: case 8: // ICONST_M1, ICONST_0 ... ICONST_5
                case 9: case 10: case 11: case 12: case 13: case 14: case 15: // LCONST_0, LCONST_1, FCONST_0, FCONST_1, FCONST_2, DCONST_0, DCONST_1
                case 26: case 27: case 28: case 29: // ILOAD_0 ... ILOAD_3
                case 30: case 31: case 32: case 33: // LLOAD_0 ... LLOAD_3
                case 34: case 35: case 36: case 37: // FLOAD_0 ... FLOAD_3
                case 38: case 39: case 40: case 41: // DLOAD_0 ... DLOAD_3
                case 42: case 43: case 44: case 45: // ALOAD_0 ... ALOAD_3
                case 89: case 90: case 91: // DUP, DUP_X1, DUP_X2
                    ++Depth;
                    break;
                case 16: case 18: // BIPUSH, LDC
                case 21: case 22: case 23: case 24: case 25: // ILOAD, LLOAD, FLOAD, DLOAD, ALOAD
                    ++Offset;
                    ++Depth;
                    break;
                case 17: // SIPUSH
                case 19: case 20: // LDC_W, LDC2_W
                case 168: // JSR
                case 178: // GETSTATIC
                case 187: // NEW
                    ++++Offset;
                    ++Depth;
                    break;
                case 46: case 47: case 48: case 49: case 50: case 51: case 52: case 53: // IALOAD, LALOAD, FALOAD, DALOAD, AALOAD, BALOAD, CALOAD, SALOAD
                case 59: case 60: case 61: case 62: // ISTORE_0 ... ISTORE_3
                case 63: case 64: case 65: case 66: // LSTORE_0 ... LSTORE_3
                case 67: case 68: case 69: case 70: // FSTORE_0 ... FSTORE_3
                case 71: case 72: case 73: case 74: // DSTORE_0 ... DSTORE_3
                case 75: case 76: case 77: case 78: // ASTORE_0 ... ASTORE_3
                case 87: // POP
                case 96: case 97: case 98: case 99:     // IADD, LADD, FADD, DADD
                case 100: case 101: case 102: case 103: // ISUB, LSUB, FSUB, DSUB
                case 104: case 105: case 106: case 107: // IMUL, LMUL, FMUL, DMUL
                case 108: case 109: case 110: case 111: // IDIV, LDIV, FDIV, DDIV
                case 112: case 113: case 114: case 115: // IREM, LREM, FREM, DREM
                case 120: case 121: // ISHL, LSHL
                case 122: case 123: // ISHR, LSHR
                case 124: case 125: // IUSHR, LUSHR
                case 126: case 127: // IAND, LAND
                case 128: case 129: // IOR, LOR
                case 130: case 131: // IXOR, LXOR
                case 148: case 149: case 150: case 151: case 152: // LCMP, FCMPL, FCMPG, DCMPL, DCMPG
                case 172: case 173: case 174: case 175: case 176: // IRETURN, LRETURN, FRETURN, DRETURN, ARETURN
                case 194: case 195: // MONITORENTER, MONITOREXIT
                    --Depth;
                    break;
                case 153: case 154: case 155: case 156: case 157: case 158: // IFEQ, IFNE, IFLT, IFGE, IFGT, IFLE
                case 179: // PUTSTATIC
                case 198: case 199: // IFNULL, IFNONNULL
                    ++++Offset;
                    --Depth;
                    break;
                case 54: case 55: case 56: case 57: case 58: // ISTORE, LSTORE, FSTORE, DSTORE, ASTORE
                    ++Offset;
                    --Depth;
                    break;
                case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: // IASTORE, LASTORE, FASTORE, DASTORE, AASTORE, BASTORE, CASTORE, SASTORE
                    ------Depth;
                    break;
                case 92: case 93: case 94: // DUP2, DUP2_X1, DUP2_X2
                    ++++Depth;
                    break;
                case 132: // IINC
                case 167: // GOTO
                case 180: // GETFIELD
                case 189: // ANEWARRAY
                case 192: // CHECKCAST
                case 193: // INSTANCEOF
                    ++++Offset;
                    break;
                case 159: case 160: case 161: case 162: case 163: case 164: case 165: case 166: // IF_ICMPEQ, IF_ICMPNE, IF_ICMPLT, IF_ICMPGE, IF_ICMPGT, IF_ICMPLE, IF_ACMPEQ, IF_ACMPNE
                case 181: // PUTFIELD
                    ++++Offset;
                    ----Depth;
                    break;
                case 88: // POP2
                    ----Depth;
                    break;
                case 169: // RET
                case 188: // NEWARRAY
                    ++Offset;
                    break;
                case 170: { // TABLESWITCH
                    Offset = (Offset + 4) & 0x00FFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    auto Reader = xel::xStreamReader(&CodeBinary[Offset]);
                    uint32_t Low = Reader.R4();
                    uint32_t High = Reader.R4();
                    Offset += 8;
                    Offset += 4 * (High - Low + 1) - 1;
                    --Depth;
                    break;
                }
                case 171: { // LOOKUPSWITCH
                    Offset = (Offset + 4) & 0xFFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    auto Reader = xel::xStreamReader(&CodeBinary[Offset]);
                    auto Count = Reader.R4();
                    Offset += 4;
                    Offset += (8 * Count) - 1;
                    --Depth;
                    break;
                }
                case 182: case 183: { // INVOKEVIRTUAL, INVOKESPECIAL
                    auto Reader = xel::xStreamReader(&CodeBinary[Offset + 1]);
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    Offset += 2;
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth -= 1 + CountMethodParameters(Descriptor); // 1 for 'this'
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    break;
                }
                case 184: { // INVOKESTATIC
                    auto Reader = xel::xStreamReader(&CodeBinary[Offset + 1]);
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    Offset += 2;
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth -= CountMethodParameters(Descriptor);
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    break;
                }
                case 185: { // INVOKEINTERFACE
                    auto Reader = xel::xStreamReader(&CodeBinary[Offset + 1]);
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    Offset += 2;
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth -= 1 + CountMethodParameters(Descriptor);
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    Offset += 2; // skip count + one byte
                    break;
                }
                case 186: { // INVOKEDYNAMIC
                    auto Reader = xel::xStreamReader(&CodeBinary[Offset + 1]);
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    Offset += 2;
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth -= CountMethodParameters(Descriptor);
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    Offset += 2; // skip count + one byte
                    break;
                }
                case 196: // WIDE
                    OpCode = CodeBinary[++Offset];
                    if (OpCode == 132) { // IINC
                        Offset += 4;
                    } else {
                        Offset += 2;
                        switch (OpCode) {
                            case 21: case 22: case 23: case 24: case 25: // ILOAD, LLOAD, FLOAD, DLOAD, ALOAD
                                ++Depth;
                                break;
                            case 54: case 55: case 56: case 57: case 58: // ISTORE, LSTORE, FSTORE, DSTORE, ASTORE
                                --Depth;
                                break;
                            case 169: // RET
                                break;
                        }
                    }
                    break;
                case 197: // MULTIANEWARRAY
                    Offset += 3;
                    Depth += 1 - CodeBinary[Offset];
                    break;
                case 201: // JSR_W
                    Offset += 4;
                    ++Depth;
                case 200: // GOTO_W
                    Offset += 4;
                    break;
                default:
                    break;
            }
        }

        X_DEBUG_PRINTF("EvalStackDepth: From:%zi, To:%zi, Depth:%zi\n", FromOffset, ToOffset, Depth);
        return Depth;
    }

    xel::ssize_t xJavaBlock::GetMinDepth() const
    {
        ssize_t Depth = 0;
        ssize_t MinDepth = Depth;

        const xJavaClass * JavaClassPtr = GetClass();
        const std::vector<xel::ubyte> & CodeBinary = *GetCode();

        auto & ClassInfo = JavaClassPtr->ClassInfo;
        auto Reader = xel::xStreamReader(CodeBinary.data());
        for (size_t Offset=this->FromOffset, ToOffset=this->ToOffset; Offset < ToOffset; Offset++) {
            auto OpCode = CodeBinary[Offset];

            switch (OpCode) {
                case 1: // ACONST_NULL
                case 2: case 3: case 4: case 5: case 6: case 7: case 8: // ICONST_M1, ICONST_0 ... ICONST_5
                case 9: case 10: case 11: case 12: case 13: case 14: case 15: // LCONST_0, LCONST_1, FCONST_0, FCONST_1, FCONST_2, DCONST_0, DCONST_1
                case 26: case 27: case 28: case 29: // ILOAD_0 ... ILOAD_3
                case 30: case 31: case 32: case 33: // LLOAD_0 ... LLOAD_3
                case 34: case 35: case 36: case 37: // FLOAD_0 ... FLOAD_3
                case 38: case 39: case 40: case 41: // DLOAD_0 ... DLOAD_3
                case 42: case 43: case 44: case 45: // ALOAD_0 ... ALOAD_3
                    Depth += 1;
                    break;
                case 89: // DUP
                    Depth -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 2;
                    break;
                case 90: // DUP_X1
                    Depth -= 2;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 3;
                    break;
                case 91: // DUP_X2
                    Depth -= 3;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 4;
                    break;
                case 16: case 18: // BIPUSH, LDC
                case 21: case 22: case 23: case 24: case 25: // ILOAD, LLOAD, FLOAD, DLOAD, ALOAD
                    Offset += 1;
                    Depth  += 1;
                    break;
                case 17: // SIPUSH
                case 19: case 20: // LDC_W, LDC2_W
                case 168: // JSR
                case 178: // GETSTATIC
                case 187: // NEW
                    Offset += 2;
                    Depth  += 1;
                    break;
                case 46: case 47: case 48: case 49: case 50: case 51: case 52: case 53: // IALOAD, LALOAD, FALOAD, DALOAD, AALOAD, BALOAD, CALOAD, SALOAD
                case 96: case 97: case 98: case 99:     // IADD, LADD, FADD, DADD
                case 100: case 101: case 102: case 103: // ISUB, LSUB, FSUB, DSUB
                case 104: case 105: case 106: case 107: // IMUL, LMUL, FMUL, DMUL
                case 108: case 109: case 110: case 111: // IDIV, LDIV, FDIV, DDIV
                case 112: case 113: case 114: case 115: // IREM, LREM, FREM, DREM
                case 120: case 121: // ISHL, LSHL
                case 122: case 123: // ISHR, LSHR
                case 124: case 125: // IUSHR, LUSHR
                case 126: case 127: // IAND, LAND
                case 128: case 129: // IOR, LOR
                case 130: case 131: // IXOR, LXOR
                case 148: case 149: case 150: case 151: case 152: // LCMP, FCMPL, FCMPG, DCMPL, DCMPG
                    Depth -= 2;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 1;
                    break;
                case 59: case 60: case 61: case 62: // ISTORE_0 ... ISTORE_3
                case 63: case 64: case 65: case 66: // LSTORE_0 ... LSTORE_3
                case 67: case 68: case 69: case 70: // FSTORE_0 ... FSTORE_3
                case 71: case 72: case 73: case 74: // DSTORE_0 ... DSTORE_3
                case 75: case 76: case 77: case 78: // ASTORE_0 ... ASTORE_3
                case 87: // POP
                case 172: case 173: case 174: case 175: case 176: // IRETURN, LRETURN, FRETURN, DRETURN, ARETURN
                case 194: case 195: // MONITORENTER, MONITOREXIT
                    Depth -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                case 153: case 154: case 155: case 156: case 157: case 158: // IFEQ, IFNE, IFLT, IFGE, IFGT, IFLE
                case 179: // PUTSTATIC
                case 198: case 199: // IFNULL, IFNONNULL
                    Offset += 2;
                    Depth  -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                case 54: case 55: case 56: case 57: case 58: // ISTORE, LSTORE, FSTORE, DSTORE, ASTORE
                    Offset += 1;
                    Depth  -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: // IASTORE, LASTORE, FASTORE, DASTORE, AASTORE, BASTORE, CASTORE, SASTORE
                    Depth -= 3;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                case 92: // DUP2
                    Depth -= 2;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 4;
                    break;
                case 93: // DUP2_X1
                    Depth -= 3;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 5;
                    break;
                case 94: // DUP2_X2
                    Depth -= 4;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 6;
                    break;
                case 132: // IINC
                case 167: // GOTO
                    Offset += 2;
                    break;
                case 180: // GETFIELD
                case 189: // ANEWARRAY
                case 192: // CHECKCAST
                case 193: // INSTANCEOF
                    Offset += 2;
                    Depth  -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 1;
                    break;
                case 159: case 160: case 161: case 162: case 163: case 164: case 165: case 166: // IF_ICMPEQ, IF_ICMPNE, IF_ICMPLT, IF_ICMPGE, IF_ICMPGT, IF_ICMPLE, IF_ACMPEQ, IF_ACMPNE
                case 181: // PUTFIELD
                    Offset += 2;
                    Depth  -= 2;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                case 88: // POP2
                    Depth -= 2;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                case 169: // RET
                    Offset += 1;
                    break;
                case 188: // NEWARRAY
                    Offset += 1;
                    Depth  -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 1;
                    break;
                case 170: { // TABLESWITCH
                    Offset = (Offset + 4) & 0xFFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    Reader.Offset(Offset);
                    uint32_t Low  = Reader.R4();
                    uint32_t High = Reader.R4();
                    Offset += 8;
                    Offset += (4 * (High - Low + 1)) - 1;
                    Depth  -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                }
                case 171: { // LOOKUPSWITCH
                    Offset = (Offset + 4) & 0xFFFC; // Skip padding
                    Offset += 4; // Skip default offset
                    Reader.Offset(Offset);
                    uint32_t Count = Reader.R4();
                    Offset += 4;
                    Offset += (8 * Count) - 1;
                    Depth  -= 1;
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    break;
                }
                case 182: case 183: { // INVOKEVIRTUAL, INVOKESPECIAL
                    Reader.Offset(Offset);
                    Offset += 2;
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth  -= 1 + CountMethodParameters(Descriptor);
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    break;
                }
                case 184: { // INVOKESTATIC
                    Reader.Offset(Offset);
                    Offset += 2;
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth  -= CountMethodParameters(Descriptor);
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    break;
                }
                case 185: { // INVOKEINTERFACE
                    Reader.Offset(Offset);
                    Offset += 2;
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth  -= 1 + CountMethodParameters(Descriptor);
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Offset += 2; // Skip 'count' and one byte
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    break;
                }
                case 186: { // INVOKEDYNAMIC
                    Reader.Offset(Offset);
                    Offset += 2;
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    Depth  -= CountMethodParameters(Descriptor);
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Offset += 2; // Skip 2 bytes
                    if (Descriptor.back() != 'V') {
                        ++Depth;
                    }
                    break;
                }
                case 196: // WIDE
                    OpCode = CodeBinary[++Offset];
                    if (OpCode == 132) { // IINC
                        Offset += 4;
                    } else {
                        Offset += 2;
                        switch (OpCode) {
                            case 21: case 22: case 23: case 24: case 25: // ILOAD, LLOAD, FLOAD, DLOAD, ALOAD
                                Depth++;
                                break;
                            case 54: case 55: case 56: case 57: case 58: // ISTORE, LSTORE, FSTORE, DSTORE, ASTORE
                                Depth--;
                                if (MinDepth > Depth) {
                                    MinDepth = Depth;
                                }
                                break;
                            case 169: // RET
                                break;
                        }
                    }
                    break;
                case 197: // MULTIANEWARRAY
                    Offset += 3;
                    Depth -= CodeBinary[Offset];
                    if (MinDepth > Depth) {
                        MinDepth = Depth;
                    }
                    Depth += 1;
                    break;
                case 201: // JSR_W
                    Offset += 4;
                    Depth  += 1;
                case 200: // GOTO_W
                    Offset += 4;
                    break;
                default:
                    break;
            }
        }

        X_DEBUG_PRINTF("GetMinDepth: From:%zi, To:%zi, Depth:%zi, MinDepth:%zi\n", FromOffset, ToOffset, Depth, MinDepth);
        return MinDepth;
    }

    /**
     * @brief sub block operations
     *
    */

    bool xJavaBlock::Contains(xJavaBlock * CheckBlockPtr) const
    {
        if (NextBlockPtr == CheckBlockPtr) {
            return true;
        }
        if (BranchBlockPtr == CheckBlockPtr) {
            return true;
        }
        for (auto & EH : ExceptionHandlers) {
            if (EH.HandlerBlockPtr == CheckBlockPtr) {
                return true;
            }
        }
        for (auto & SC : SwitchCases) {
            if (SC.BlockPtr == CheckBlockPtr) {
                return true;
            }
        }
        if (FirstSubBlockPtr == CheckBlockPtr) {
            return true;
        }
        if (SecondSubBlockPtr == CheckBlockPtr) {
            return true;
        }
        return false;
    }

    /**************************************
     *
     *
     *
     *
     * **********************************/

    /**
     * @brief not confirmed codes:
     *
     */


    void xJavaBlock::Replace(xJavaBlock * OldBlockPtr, xJavaBlock * NewBlockPtr)
    {
        assert(OldBlockPtr);
        assert(NewBlockPtr);
        if (NextBlockPtr == OldBlockPtr) {
            NextBlockPtr = NewBlockPtr;
        }
        if (BranchBlockPtr == OldBlockPtr) {
            BranchBlockPtr = NewBlockPtr;
        }
        for (auto & EH : ExceptionHandlers) {
            EH.HandlerBlockPtr->Replace(OldBlockPtr, NewBlockPtr);
        }
        for (auto & SC : SwitchCases) {
            SC.BlockPtr->Replace(OldBlockPtr, NewBlockPtr);
        }
        if (FirstSubBlockPtr == OldBlockPtr) {
            FirstSubBlockPtr = NewBlockPtr;
        }
        if (SecondSubBlockPtr == OldBlockPtr) {
            SecondSubBlockPtr = NewBlockPtr;
        }

        auto Iter = Predecessors.find(OldBlockPtr);
        if (Iter != Predecessors.end()) {
            Predecessors.erase(Iter);
            if (NewBlockPtr->Type != xJavaBlock::TYPE_END) {
                Predecessors.insert(NewBlockPtr);
            }
        }
    }

    void xJavaBlock::AddExceptionHandler(const xJavaExceptionHandler & ExceptionHandler)
    {
    #ifndef NDEBUG
        for(auto & Handler : ExceptionHandlers) {
            if (Handler.FixedCatchTypeName == ExceptionHandler.FixedCatchTypeName) {
                xel::Fatal("Duplicate Exception catch type");
            }
        }
    #endif
        ExceptionHandlers.push_back(ExceptionHandler);
    }

    void xJavaBlock::InverseCondition()
    {
        switch (Type) {
            case TYPE_CONDITION:
            case TYPE_CONDITION_TERNARY_OPERATOR:
            case TYPE_GOTO_IN_TERNARY_OPERATOR:
                MustInverseCondition ^= true;
                break;
            case TYPE_CONDITION_AND:
                Type = TYPE_CONDITION_OR;
                FirstSubBlockPtr->InverseCondition();
                SecondSubBlockPtr->InverseCondition();
                break;
            case TYPE_CONDITION_OR:
                Type = TYPE_CONDITION_AND;
                FirstSubBlockPtr->InverseCondition();
                SecondSubBlockPtr->InverseCondition();
                break;
            default:
                xel::Fatal("Invalid condition");
                break;
        }
    }

    xJavaSwitchCase::xJavaSwitchCase(xJavaBlock * BlockPtr)
    : DefaultCase(true), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

    xJavaSwitchCase::xJavaSwitchCase(size_t Value, xJavaBlock * BlockPtr)
    : DefaultCase(false), Value(Value), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

}
