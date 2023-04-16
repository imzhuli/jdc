#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <xel/String.hpp>

namespace jdc
{

    std::unique_ptr<xJavaControlFlowGraph> xJavaControlFlowGraph::ParseByteCode(const xJavaMethod * JavaMethodPtr)
    {
        auto JavaControlFlowGraphUPtr = std::make_unique<xJavaControlFlowGraph>();
        auto CFGPtr = JavaControlFlowGraphUPtr.get();

        if (!CFGPtr->Init(JavaMethodPtr)) {
            return {};
        }
        return JavaControlFlowGraphUPtr;
    }

    bool xJavaControlFlowGraph::Init(const xJavaMethod * JavaMethodPtr)
    {
        _JavaMethodPtr = JavaMethodPtr;
        _JavaClassPtr = JavaMethodPtr->JavaClassPtr;

        InitLocalVariables();
        InitBlocks();

        return true;
    }

    void xJavaControlFlowGraph::InitLocalVariables()
    {
        xel::Renew(LocalVariableList);

        if (_JavaMethodPtr->MethodInfoPtr->AccessFlags & ACC_STATIC) {
            // no "this" parameter
        }
        else {
            LocalVariableList.push_back({ _JavaClassPtr->GetInnermostName(), "this"s });
            ++FirstVariableIndex;
        }

        if (_JavaMethodPtr->IsConstructor) {
            if (_JavaClassPtr->IsEnum()) {
                // TODO: enum constructor
            }
            else {
                if (_JavaClassPtr->IsInnerClass()) {
                    // add local variable this$0:
                    LocalVariableList.push_back({ _JavaClassPtr->Extend.OuterClassPtr->GetFixedCodeName(), "this$0"s });
                }
            }
        }

        X_DEBUG_PRINTF("JavaMethod: %s.%s local variables:\n", _JavaClassPtr->GetFixedCodeName().c_str(), _JavaMethodPtr->OriginalName.c_str());
        for (auto & Variable : LocalVariableList) {
            X_DEBUG_PRINTF("TypeCodeName: %s VariableName:%s\n", Variable.TypeCodeName.c_str(), Variable.VariableName.c_str());
        }
    }

    enum eCodeType : uint8_t
    {
        CT_UNKNOWN = 0,
        CT_GOTO,         // 'g'
        CT_TERNARY_GOTO, // 'G'
        CT_THROW,        // 't'
        CT_RETURN,       // 'r'
        CT_CONDITIONAL,  // 'c'
        CT_SWITCH,       // 's'
        CT_JSR,          // 'j'
        CT_RET,          // 'R'
        CT_RETURN_VALUE, // 'v'

        // default:
        CT_STATEMENT
    };

    static bool IsILOADForIINC(const std::vector<xel::ubyte> & code, size_t offset, size_t index) {
        if (++offset < code.size()) {
            xOpCode nextOpcode = code[offset];
            if (nextOpcode == OP_ILOAD) { // ILOAD
                if (index == (code[offset+1])) {
                    return true;
                }
            } else if (nextOpcode == 26+index) { // ILOAD_0 ... ILOAD_3
                return true;
            }
        }
        return false;
    }

    void xJavaControlFlowGraph::InitBlocks()
    {
        xel::Renew(Blocks);
        xel::Renew(BlockList);
        FirstVariableIndex = 0;

        auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(_JavaMethodPtr->Converted.AttributeMap, "Code");
        auto & CodeBinary = CodeAttributePtr->CodeBinary;

        size_t CodeLength = CodeBinary.size();
        BlockList.resize(CodeLength);

        auto CodeTypes = std::vector<eCodeType>(CodeLength);
        auto NextOffsets = std::vector<size_t>(CodeLength);
        auto BranchOffsets = std::vector<size_t>(CodeLength);;
        auto SwitchValues = std::vector<std::vector<size_t>>(CodeLength);
        auto SwitchOffsets = std::vector<std::vector<size_t>>(CodeLength);

        auto MARK = xJavaBlock::TYPE_END;
        Blocks[0].Type = MARK;

        auto LastOffset = size_t(0);
        auto LastStatementOffset = size_t(-1);
        auto Reader = xel::xStreamReader(CodeBinary.data());
        for (size_t Offset = 0 ; Offset < CodeLength; ++Offset) {
            NextOffsets[LastOffset] = Offset;
            LastOffset = Offset;

            auto OpCode = (xOpCode)CodeBinary[Offset];
            switch(OpCode) {
                case OP_BIPUSH:
                case OP_LDC:
                case OP_ILOAD: case OP_LLOAD: case OP_FLOAD: case OP_DLOAD: case OP_ALOAD:
                case OP_NEWARRAY:
                    ++Offset;
                    break;
                case OP_ISTORE: case OP_LSTORE: case OP_FSTORE: case OP_DSTORE: case OP_ASTORE:
                    ++Offset;
                    LastStatementOffset = Offset;
                    break;
                case OP_ISTORE_0: case OP_ISTORE_1: case OP_ISTORE_2: case OP_ISTORE_3:
                case OP_LSTORE_0: case OP_LSTORE_1: case OP_LSTORE_2: case OP_LSTORE_3:
                case OP_FSTORE_0: case OP_FSTORE_1: case OP_FSTORE_2: case OP_FSTORE_3:
                case OP_DSTORE_0: case OP_DSTORE_1: case OP_DSTORE_2: case OP_DSTORE_3:
                case OP_ASTORE_0: case OP_ASTORE_1: case OP_ASTORE_2: case OP_ASTORE_3:
                case OP_IASTORE:  case OP_LASTORE:  case OP_FASTORE:  case OP_DASTORE: case OP_AASTORE: case OP_BASTORE: case OP_CASTORE: case OP_SASTORE:
                case OP_POP:      case OP_POP2:
                case OP_MONITORENTER:
                case OP_MONITOREXIT:
                    LastStatementOffset = Offset;
                    break;
                case OP_RET:
                    ++Offset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    CodeTypes[Offset] = CT_RETURN;
                    if (Offset + 1 < CodeLength) {
                        Blocks[Offset + 1].Type = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                case OP_PUTSTATIC: case OP_PUTFIELD:
                    Offset += 2;
                    LastStatementOffset = Offset;
                    break;
                case OP_INVOKEVIRTUAL: case OP_INVOKESPECIAL: case OP_INVOKESTATIC: {
                    Reader.Offset(Offset);
                    Offset += 2;
                    auto & ClassInfo = _JavaClassPtr->ClassInfo;
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    if (Descriptor.back() == 'V') {
                        LastStatementOffset = Offset;
                    }
                    break;
                }
                case OP_INVOKEINTERFACE: case OP_INVOKEDYNAMIC:{
                    Reader.Offset(Offset);
                    Offset += 4; // skip 2 extra bytes
                    auto & ClassInfo = _JavaClassPtr->ClassInfo;
                    auto & ConstantRef = ClassInfo.GetConstantInfo(Reader.R2());
                    assert(ConstantRef.Tag == eConstantTag::MethodRef);
                    auto & NameAndType = ClassInfo.GetConstantInfo(ConstantRef.Details.MethodRef.NameAndTypeIndex);
                    auto & Descriptor = ClassInfo.GetConstantUtf8(NameAndType.Details.NameAndType.DescriptorIndex);
                    if (Descriptor.back() == 'V') {
                        LastStatementOffset = Offset;
                    }
                    break;
                }
                case OP_IINC:
                    Offset += 2;
                    if ((LastStatementOffset + 3 == Offset) && !IsILOADForIINC(CodeBinary, Offset, (size_t)CodeBinary[Offset - 1])) {
                        // Last instruction is a 'statement' & the next instruction is not a matching ILOAD -> IINC as a statement
                        LastStatementOffset = Offset;
                    }
                    break;
                case OP_SIPUSH:
                case OP_LDC_W: case OP_LDC2_W:
                case OP_GETSTATIC: case OP_GETFIELD:
                case OP_NEW: case OP_ANEWARRAY:
                case OP_CHECKCAST:
                case OP_INSTANCEOF:
                    Offset += 2;
                    break;
                case OP_GOTO: {
                    eCodeType CodeType = (Offset == LastStatementOffset + 1) ? CT_GOTO : CT_TERNARY_GOTO;
                    if (LastStatementOffset != size_t(-1)) {
                        Blocks[LastStatementOffset + 1].Type = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    Reader.Offset(Offset);
                    Offset += 2;
                    CodeTypes[Offset] = CodeType;
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Blocks[BranchOffset].Type = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        Blocks[Offset + 1].Type = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
                case OP_JSR: {
                    if (LastStatementOffset != size_t(-1)) {
                        Blocks[LastStatementOffset + 1].Type = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    Reader.Offset(Offset);
                    Offset += 2;
                    CodeTypes[Offset] = CT_JSR;
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Blocks[BranchOffset].Type = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        Blocks[Offset + 1].Type = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
                case OP_IFEQ:      case OP_IFNE:      case OP_IFLT:      case OP_IFGE:      case OP_IFGT:      case OP_IFLE:
                case OP_IF_ICMPEQ: case OP_IF_ICMPNE: case OP_IF_ICMPLT: case OP_IF_ICMPGE: case OP_IF_ICMPGT: case OP_IF_ICMPLE: case OP_IF_ACMPEQ: case OP_IF_ACMPNE:
                case OP_IFNULL:    case OP_IFNONNULL: {
                    if (LastStatementOffset != size_t(-1)) {
                        Blocks[LastStatementOffset + 1].Type = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    Reader.Offset(Offset);
                    Offset += 2;
                    CodeTypes[Offset] = CT_CONDITIONAL;
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Blocks[BranchOffset].Type = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        Blocks[Offset + 1].Type = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }




                // TODO: other cases
            }

        }

        (void) CodeBinary;
    }

}
