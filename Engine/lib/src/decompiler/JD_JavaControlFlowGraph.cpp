#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <xel/String.hpp>

namespace jdc
{

    xJavaSwitchCase::xJavaSwitchCase(xJavaBlock * BlockPtr)
    : DefaultCase(true), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

    xJavaSwitchCase::xJavaSwitchCase(size_t Value, xJavaBlock * BlockPtr)
    : DefaultCase(false), Value(Value), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

    int CodeExceptionComparator(const xJavaException & E1, const xJavaException & E2)
    {
        if (E1.StartPC == E2.StartPC) {
            if (E1.EndPC == E2.EndPC) {
                return 0;
            }
            if (E1.EndPC < E2.EndPC) {
                return -1;
            }
            return 1;
        }
        if (E1.StartPC < E2.StartPC) {
            return -1;
        }
        return 1;
    }

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
        xel::Renew(BlockList);
        FirstVariableIndex = 0;

        auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(_JavaMethodPtr->Converted.AttributeMap, "Code");
        auto & CodeBinary = CodeAttributePtr->CodeBinary;

        size_t CodeLength = CodeBinary.size();

        auto BlockTypes = std::vector<xJavaBlock::eType>(CodeLength);
        auto CodeTypes = std::vector<eCodeType>(CodeLength);
        auto NextOffsets = std::vector<size_t>(CodeLength);
        auto BranchOffsets = std::vector<size_t>(CodeLength);;
        auto SwitchValueTable = std::vector<std::vector<size_t>>(CodeLength);
        auto SwitchOffsetTable = std::vector<std::vector<size_t>>(CodeLength);

        auto MARK = xJavaBlock::TYPE_END;
        BlockTypes[0] = MARK;

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
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                case OP_PUTSTATIC: case OP_PUTFIELD:
                    Offset += 2;
                    LastStatementOffset = Offset;
                    break;
                case OP_INVOKEVIRTUAL: case OP_INVOKESPECIAL: case OP_INVOKESTATIC: {
                    Reader.Offset(Offset + 1);
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
                    Reader.Offset(Offset + 1);
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
                        BlockTypes[LastStatementOffset + 1] = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Offset += 2;
                    CodeTypes[Offset] = CodeType;
                    BlockTypes[BranchOffset] = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
                case OP_JSR: {
                    if (LastStatementOffset != size_t(-1)) {
                        BlockTypes[LastStatementOffset + 1] = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    CodeTypes[Offset] = CT_JSR;
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Offset += 2;
                    CodeTypes[Offset] = CT_JSR;
                    BlockTypes[BranchOffset] = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
                case OP_IFEQ:      case OP_IFNE:      case OP_IFLT:      case OP_IFGE:      case OP_IFGT:      case OP_IFLE:
                case OP_IF_ICMPEQ: case OP_IF_ICMPNE: case OP_IF_ICMPLT: case OP_IF_ICMPGE: case OP_IF_ICMPGT: case OP_IF_ICMPLE: case OP_IF_ACMPEQ: case OP_IF_ACMPNE:
                case OP_IFNULL:    case OP_IFNONNULL: {
                    if (LastStatementOffset != size_t(-1)) {
                        BlockTypes[LastStatementOffset + 1] = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Offset += 2;
                    CodeTypes[Offset] = CT_CONDITIONAL;
                    BlockTypes[BranchOffset] = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }

                case OP_TABLESWITCH: {
                    // skip padding:
                    Reader.Offset((Offset + 4) & 0x00FFFC);
                    size_t DefaultOffset = Offset + Reader.R4();
                    BlockTypes[DefaultOffset] = MARK;

                    size_t Low = Reader.R4();
                    size_t High = Reader.R4();
                    auto Values  = std::vector<size_t>(High - Low + 2);
                    auto Offsets = std::vector<size_t>(High - Low + 2);

                    Offsets[0] = DefaultOffset;

                    for (size_t I = 1, Len = High - Low + 2; I < Len; I++) {
                        Values[I] = Low + I - 1;
                        size_t BranchOffset = Offsets[I] = Offset + Reader.R4();
                        BlockTypes[BranchOffset] = MARK;
                    }
                    Offset = Reader.Offset() - 1;
                    CodeTypes[Offset] = CT_SWITCH;
                    SwitchValueTable[Offset] = std::move(Values);
                    SwitchOffsetTable[Offset] = std::move(Offsets);
                    LastStatementOffset = Offset;
                    break;
                }

                case OP_IRETURN: case OP_LRETURN: case OP_FRETURN: case OP_DRETURN: case OP_ARETURN:
                    CodeTypes[Offset] = CT_RETURN_VALUE;
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;

                case OP_RETURN:
                    if (LastStatementOffset != (size_t)-1) {
                        BlockTypes[LastStatementOffset + 1] = MARK;
                    }
                    CodeTypes[Offset] = CT_RETURN;
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;

                case OP_ATHROW:
                    CodeTypes[Offset] = CT_THROW;
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;

                case OP_WIDE: {
                    OpCode = CodeBinary[++Offset];
                    switch (OpCode) {
                        case OP_IINC: {
                            Reader.Offset(Offset + 1);
                            Offset += 4;
                            if ((LastStatementOffset + 6 == Offset) && !IsILOADForIINC(CodeBinary, Offset, Reader.R2())) {
                                // Last instruction is a 'statement' & the next instruction is not a matching ILOAD -> IINC as a statement
                                LastStatementOffset = Offset;
                            }
                            break;
                        }
                        case OP_RET: {
                            Offset += 2;
                            CodeTypes[Offset] = CT_RET;
                            if (Offset + 1 < CodeLength) {
                                BlockTypes[Offset + 1] = MARK;
                            }
                            LastStatementOffset = Offset;
                            break;
                        }
                        case OP_ISTORE: case OP_LSTORE: case OP_FSTORE: case OP_DSTORE: case OP_ASTORE: // ISTORE, LSTORE, FSTORE, DSTORE, ASTORE
                            Offset += 2;
                            LastStatementOffset = Offset;
                            break;
                        default:
                            Offset += 2;
                            break;
                    }
                    break;
                }

                case OP_MULTIANEWARRAY:
                    Offset += 3;
                    break;

                case OP_GOTO_W: {
                    eCodeType CodeType = (Offset == LastStatementOffset + 1) ? CT_GOTO : CT_TERNARY_GOTO;
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + Reader.R4();
                    Offset += 4;

                    BlockTypes[BranchOffset] = MARK;
                    CodeTypes[Offset] = CodeType;
                    BranchOffsets[Offset] = BranchOffset;

                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }

                case OP_JSR_W: {
                    if (LastStatementOffset != (size_t)-1) {
                        BlockTypes[LastStatementOffset + 1] = MARK;
                    }
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + Reader.R4();
                    Offset += 4;

                    BlockTypes[BranchOffset] = MARK;
                    CodeTypes[Offset] = CT_JSR;
                    BranchOffsets[Offset] = BranchOffset;

                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockTypes[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
            } // end of switch(opcode)
        } // end of for
        NextOffsets[LastOffset] = CodeLength;

        X_DEBUG_PRINTF("End of init blocks round 0: mark block types\n");

        auto & ExceptionTable = CodeAttributePtr->ExceptionTable;
        if (ExceptionTable.size()) {
            for (auto & Record : ExceptionTable) {
                BlockTypes[Record.StartPC] = MARK;
                BlockTypes[Record.HandlerPC] = MARK;
            }
        }

        // Create basic blocks:
        BlockList.push_back(std::make_unique<xJavaBlock>(xJavaBlock::TYPE_START, 0, 0));
        LastOffset = 0;
        auto Blocks = std::vector<xJavaBlock*>(CodeLength);
        for (size_t Offset = NextOffsets[0]; Offset < CodeLength; Offset = NextOffsets[Offset]) {
            if ((BlockTypes[Offset] != xJavaBlock::TYPE_DELETED)) {
                BlockList.push_back(std::make_unique<xJavaBlock>(LastOffset, Offset));
                Blocks[LastOffset] = BlockList.back().get();
                LastOffset = Offset;
            }
        }
        BlockList.push_back(std::make_unique<xJavaBlock>(LastOffset, CodeLength));
        Blocks[LastOffset] = BlockList.back().get();
        BlockList.push_back(std::make_unique<xJavaBlock>(xJavaBlock::TYPE_END, 0, 0));

        // set block types:
        auto MainFlowBlocks = std::vector<xJavaBlock*>();
        auto StartBlockPtr = BlockList[0].get();
        auto SuccessBlockPtr = BlockList[1].get();
        auto EndBlockPtr = BlockList.back().get();

        StartBlockPtr->NextBlockPtr = SuccessBlockPtr;
        SuccessBlockPtr->Predecessors.push_back(StartBlockPtr);
        auto BlockListLength = BlockList.size() - 1; // ignore end block
        for (size_t I = 1; I < BlockListLength; ++I) {
            auto BlockPtr = BlockList[I].get();
            size_t LastInstructionOffset = BlockPtr->ToOffset - 1;

            switch(CodeTypes[LastInstructionOffset]) {
                case CT_GOTO:
                    BlockPtr->Type = xJavaBlock::TYPE_GOTO;
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.push_back(BlockPtr);
                    break;
                case CT_TERNARY_GOTO:
                    BlockPtr->Type = xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR;
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.push_back(BlockPtr);
                    break;
                case CT_THROW:
                    BlockPtr->Type = xJavaBlock::TYPE_THROW;
                    BlockPtr->NextBlockPtr = EndBlockPtr;
                    break;
                case CT_RETURN:
                    BlockPtr->Type = xJavaBlock::TYPE_RETURN;
                    BlockPtr->NextBlockPtr = EndBlockPtr;
                    break;
                case CT_CONDITIONAL:
                    BlockPtr->Type = xJavaBlock::TYPE_CONDITIONAL_BRANCH;
                    SuccessBlockPtr = Blocks[BlockPtr->ToOffset];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.push_back(BlockPtr);
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->BranchBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.push_back(BlockPtr);
                    break;
                case CT_SWITCH: {
                    BlockPtr->Type = xJavaBlock::TYPE_SWITCH_DECLARATION;
                    auto & Values = SwitchValueTable[LastInstructionOffset];
                    auto & Offsets = SwitchOffsetTable[LastInstructionOffset];
                    auto & SwitchCases = BlockPtr->SwitchCases;

                    size_t DefaultCaseOffset = Offsets[0];
                    auto CaseBlockPtr = Blocks[DefaultCaseOffset];
                    SwitchCases.emplace_back(BlockPtr);
                    CaseBlockPtr->Predecessors.push_back(BlockPtr);

                    for (size_t Index = 1; Index < Offsets.size(); ++Index) {
                        size_t CaseOffset = Offsets[Index];
                        // TODO: DEBUG
                        // if (CaseOffset != DefaultCaseOffset) {
                            CaseBlockPtr = Blocks[CaseOffset];
                            SwitchCases.emplace_back(Values[Index], CaseBlockPtr);
                            CaseBlockPtr->Predecessors.push_back(BlockPtr);
                        // }
                    }
                    break;
                }
                case CT_JSR:
                    BlockPtr->Type = xJavaBlock::TYPE_JSR;
                    SuccessBlockPtr = Blocks[BlockPtr->ToOffset];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.push_back(BlockPtr);
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->BranchBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.push_back(BlockPtr);
                    break;
                case CT_RET:
                    BlockPtr->Type = xJavaBlock::TYPE_RET;
                    BlockPtr->NextBlockPtr = EndBlockPtr;
                    break;
                case CT_RETURN_VALUE:
                    BlockPtr->Type = xJavaBlock::TYPE_RETURN_VALUE;
                    BlockPtr->NextBlockPtr = EndBlockPtr;
                    break;
                default:
                    BlockPtr->Type = xJavaBlock::TYPE_STATEMENTS;
                    SuccessBlockPtr = Blocks[BlockPtr->ToOffset];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.push_back(BlockPtr);
                    MainFlowBlocks.push_back(BlockPtr);
                    break;
            }
        }

        X_DEBUG_BREAKPOINT();

    }

}
