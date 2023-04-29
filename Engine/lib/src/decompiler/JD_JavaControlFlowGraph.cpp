#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <xel/String.hpp>
#include <algorithm>

namespace jdc
{

    xJavaBlock xJavaControlFlowGraph::EndBlock = xJavaBlock(xJavaBlock::TYPE_END, 0, 0);

    xJavaSwitchCase::xJavaSwitchCase(xJavaBlock * BlockPtr)
    : DefaultCase(true), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

    xJavaSwitchCase::xJavaSwitchCase(size_t Value, xJavaBlock * BlockPtr)
    : DefaultCase(false), Value(Value), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

    class xCodeExceptionComparator
    {
    public:
        bool operator () (const xJavaException & E1, const xJavaException & E2) const
        {
            if (E1.StartPC == E2.StartPC) {
                return E1.EndPC < E2.EndPC;
            }
            return E1.StartPC < E2.StartPC;
        }
    };

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

        assert(LocalVariableList.empty());
        xel::Renew(FirstVariableIndex);
        assert(Blocks.empty());
        assert(BlockList.empty());

        InitLocalVariables();
        InitBlocks();
        // ReduceGoto(); in jd-core
        // ReduceLoop(); in jd-core
        ReduceGraph();

        return true;
    }

    void xJavaControlFlowGraph::Clean()
    {
        xel::Renew(FirstVariableIndex);
        xel::Renew(LocalVariableList);
        xel::Renew(Blocks);
        xel::Renew(BlockList);

        _JavaClassPtr = nullptr;
        _JavaMethodPtr = nullptr;
    }

    void xJavaControlFlowGraph::InitLocalVariables()
    {
        xel::Renew(LocalVariableList);

        if (_JavaMethodPtr->MethodInfoPtr->AccessFlags & ACC_STATIC) { // no "this" parameter
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
        CT_TRY,          // 'T'

        // default:
        CT_STATEMENT     // TYPE_STATEMENTS
    };

    static bool IsILOADForIINC(const std::vector<xel::ubyte> & code, size_t offset, int16_t index) {
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
                            if ((LastStatementOffset + 6 == Offset) && !IsILOADForIINC(CodeBinary, Offset, (int16_t)Reader.R2())) {
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

        auto & ExceptionTable = CodeAttributePtr->ExceptionTable;
        if (ExceptionTable.size()) {
            for (auto & Mark : ExceptionTable) {
                BlockTypes[Mark.StartPC] = MARK;
                BlockTypes[Mark.HandlerPC] = MARK;
            }
        }

        // Create basic blocks:
        BlockList.push_back(std::make_unique<xJavaBlock>(xJavaBlock::TYPE_START, 0, 0));
        Blocks.resize(CodeLength);
        LastOffset = 0;
        for (size_t Offset = NextOffsets[0]; Offset < CodeLength; Offset = NextOffsets[Offset]) {
            if ((BlockTypes[Offset] != xJavaBlock::TYPE_DELETED)) {
                BlockList.push_back(std::make_unique<xJavaBlock>(LastOffset, Offset));
                Blocks[LastOffset] = BlockList.back().get();
                LastOffset = Offset;
            }
        }
        BlockList.push_back(std::make_unique<xJavaBlock>(LastOffset, CodeLength));
        Blocks[LastOffset] = BlockList.back().get();

        // set block types:
        auto MainFlowBlocks = std::vector<xJavaBlock*>();
        auto StartBlockPtr = BlockList[0].get();
        auto SuccessBlockPtr = BlockList[1].get();

        StartBlockPtr->NextBlockPtr = SuccessBlockPtr;
        SuccessBlockPtr->Predecessors.insert(StartBlockPtr);
        auto BlockListLength = BlockList.size() - 1; // ignore end block
        for (size_t I = 1; I < BlockListLength; ++I) {
            auto BlockPtr = BlockList[I].get();
            size_t LastInstructionOffset = BlockPtr->ToOffset - 1;

            switch(CodeTypes[LastInstructionOffset]) {
                case CT_GOTO:
                    BlockPtr->Type = xJavaBlock::TYPE_GOTO;
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    break;
                case CT_TERNARY_GOTO:
                    BlockPtr->Type = xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR;
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
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
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->BranchBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    break;
                case CT_SWITCH: {
                    BlockPtr->Type = xJavaBlock::TYPE_SWITCH_DECLARATION;
                    auto & Values = SwitchValueTable[LastInstructionOffset];
                    auto & Offsets = SwitchOffsetTable[LastInstructionOffset];
                    auto & SwitchCases = BlockPtr->SwitchCases;

                    size_t DefaultCaseOffset = Offsets[0];
                    auto CaseBlockPtr = Blocks[DefaultCaseOffset];
                    SwitchCases.emplace_back(BlockPtr);
                    CaseBlockPtr->Predecessors.insert(BlockPtr);

                    for (size_t Index = 1; Index < Offsets.size(); ++Index) {
                        size_t CaseOffset = Offsets[Index];
                        // if (CaseOffset != DefaultCaseOffset) { // merge case shared with default
                            CaseBlockPtr = Blocks[CaseOffset];
                            SwitchCases.emplace_back(Values[Index], CaseBlockPtr);
                            CaseBlockPtr->Predecessors.insert(BlockPtr);
                        // }
                    }
                    break;
                }
                case CT_JSR:
                    BlockPtr->Type = xJavaBlock::TYPE_JSR;
                    SuccessBlockPtr = Blocks[BlockPtr->ToOffset];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    SuccessBlockPtr = Blocks[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->BranchBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
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
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    MainFlowBlocks.push_back(BlockPtr);
                    break;
            }
        }

        if (ExceptionTable.size()) {
            // copy & sort code exceptions byte StartPC & EndPC
            std::vector<xJavaException> CodeExceptions;
            for (auto & Mark : ExceptionTable) {
                auto NewCodeException = xJavaException {
                    .StartPC   = Mark.StartPC,
                    .EndPC     = Mark.EndPC,
                    .HandlerPC = Mark.HandlerPC,
                    .CatchType = Mark.CatchType,
                };
                CodeExceptions.push_back(NewCodeException);
            }
            std::sort(CodeExceptions.begin(), CodeExceptions.end(), xCodeExceptionComparator());

            auto Cache = std::map<xJavaException, xJavaBlock*, xCodeExceptionComparator>();
            auto & HandlePCToStartPC = BranchOffsets;
            auto & HandlePCMarks = CodeTypes;
            for (auto & CE : CodeExceptions) {
                auto StartPC = CE.StartPC;
                auto EndPC = CE.EndPC;
                auto HandlerPC = CE.HandlerPC;

                if (StartPC == HandlerPC) {
                    X_DEBUG_BREAKPOINT();
                    continue;
                }
                if (HandlePCMarks[HandlerPC] == CT_THROW && StartPC > Blocks[HandlePCToStartPC[HandlerPC]]->FromOffset) {
                    X_DEBUG_BREAKPOINT();
                    continue;
                }

                auto & TryCatchFinalBlockPtr = Cache[CE];
                if (!TryCatchFinalBlockPtr) {
                    // Insert a new 'try-catch-finally' basic block
                    auto StartBlockPtr = Blocks[StartPC];
                    BlockList.push_back(std::make_unique<xJavaBlock>(xJavaBlock::TYPE_TRY_DECLARATION, StartPC, EndPC));
                    TryCatchFinalBlockPtr = BlockList.back().get();
                    TryCatchFinalBlockPtr->NextBlockPtr = StartBlockPtr;

                    // Update predecessors
                    auto & StartBlockPredecessors = StartBlockPtr->Predecessors;
                    for (auto Iter = StartBlockPredecessors.begin(); Iter != StartBlockPredecessors.end();) {
                        auto & PredecessorPtr = *Iter;
                        if (!StartBlockPtr->Contains(PredecessorPtr)) {
                            PredecessorPtr->Replace(StartBlockPtr, TryCatchFinalBlockPtr);
                            TryCatchFinalBlockPtr->Predecessors.insert(PredecessorPtr);
                            Iter = StartBlockPredecessors.erase(Iter);
                        } else {
                            ++Iter;
                        }
                    }

                    StartBlockPredecessors.insert(TryCatchFinalBlockPtr);
                    Blocks[StartPC] = TryCatchFinalBlockPtr;
                    Cache[CE] = TryCatchFinalBlockPtr;
                }

                if (CE.CatchType) {
                    auto & UnfixedTypeBinaryName = _JavaClassPtr->ClassInfo.GetConstantClassBinaryName(CE.CatchType);
                    CE.FixedExceptionClassBinaryName = _JavaClassPtr->JavaSpacePtr->GetFixedClassBinaryName(UnfixedTypeBinaryName);

                    X_DEBUG_PRINTF("ExceptionClassBinaryName: %s\n", CE.FixedExceptionClassBinaryName.c_str());
                }

                auto HandlerBlockPtr = Blocks[HandlerPC];
                auto Handler = xJavaExceptionHandler{ CE.FixedExceptionClassBinaryName, HandlerBlockPtr };
                TryCatchFinalBlockPtr->AddExceptionHandler(Handler);

                HandlerBlockPtr->Predecessors.insert(TryCatchFinalBlockPtr);
                HandlePCToStartPC[HandlerPC] = StartPC;
                HandlePCMarks[HandlerPC] = CT_THROW;
            }
        }

        // --- Recheck TYPE_GOTO_IN_TERNARY_OPERATOR --- //
        for (auto BlockPtr : Blocks) {
            if (!BlockPtr) {
                continue;
            }
            auto NextBlockPtr = BlockPtr->NextBlockPtr;
            auto PredecessorsPtr = static_cast<std::set<xJavaBlock *>*>(nullptr);

            if (BlockPtr->Type != xJavaBlock::TYPE_STATEMENTS) {
                continue;
            }
            if (NextBlockPtr->Predecessors.size() != 1) {
                continue;
            }

            if (NextBlockPtr->Type == xJavaBlock::TYPE_GOTO && EvalStackDepth(_JavaClassPtr, CodeBinary, BlockPtr)) {
                // Transform STATEMENTS and GOTO to GOTO_IN_TERNARY_OPERATOR
                BlockPtr->Type = xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR;
                BlockPtr->ToOffset = NextBlockPtr->ToOffset;

                BlockPtr->NextBlockPtr = NextBlockPtr->NextBlockPtr;
                PredecessorsPtr = & NextBlockPtr->NextBlockPtr->Predecessors;
                PredecessorsPtr->erase(NextBlockPtr);
                PredecessorsPtr->insert(BlockPtr);

                NextBlockPtr->Type = xJavaBlock::TYPE_DELETED;
            }
            else if (NextBlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH && EvalStackDepth(_JavaClassPtr, CodeBinary, BlockPtr)) {
                // Merge STATEMENTS and CONDITIONAL_BRANCH
                BlockPtr->Type = xJavaBlock::TYPE_CONDITIONAL_BRANCH;
                BlockPtr->ToOffset = NextBlockPtr->ToOffset;

                BlockPtr->NextBlockPtr = NextBlockPtr->NextBlockPtr;
                PredecessorsPtr = & NextBlockPtr->NextBlockPtr->Predecessors;
                PredecessorsPtr->erase(NextBlockPtr);
                PredecessorsPtr->insert(BlockPtr);

                BlockPtr->BranchBlockPtr = NextBlockPtr->BranchBlockPtr;
                PredecessorsPtr = & NextBlockPtr->BranchBlockPtr->Predecessors;
                PredecessorsPtr->erase(NextBlockPtr);
                PredecessorsPtr->insert(BlockPtr);

                NextBlockPtr->Type = xJavaBlock::TYPE_DELETED;
            }
        }
    }

    xOpCode xJavaControlFlowGraph::SearchNextOpcode(const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr, size_t MaxOffset)
    {
        size_t Offset   = BlockPtr->FromOffset;
        size_t ToOffset = BlockPtr->ToOffset;

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

    xOpCode xJavaControlFlowGraph::GetLastOpcode(const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr)
    {
        size_t Offset = BlockPtr->FromOffset;
        size_t ToOffset = BlockPtr->ToOffset;
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

    size_t xJavaControlFlowGraph::EvalStackDepth(const xJavaClass * JavaClassPtr, const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr)
    {
        ssize_t Depth = 0;
        auto & ClassInfo = JavaClassPtr->ClassInfo;
        for (size_t Offset = BlockPtr->FromOffset, ToOffset = BlockPtr->ToOffset; Offset < ToOffset; ++Offset) {
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

        X_DEBUG_PRINTF("xJavaControlFlowGraph::EvalStackDepth: FromOffset:%zi, ToOffset:%zi, Depth:%zi\n", BlockPtr->FromOffset, BlockPtr->ToOffset, Depth);
        return Depth;
    }

    size_t xJavaControlFlowGraph::GetMinDepth(const xJavaClass * JavaClassPtr, const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr)
    {
        ssize_t Depth = 0;
        ssize_t MinDepth = 0;

        auto & ClassInfo = JavaClassPtr->ClassInfo;
        auto Reader = xel::xStreamReader(CodeBinary.data());
        for (size_t Offset=BlockPtr->FromOffset, ToOffset=BlockPtr->ToOffset; Offset < ToOffset; Offset++) {
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

        X_DEBUG_PRINTF("xJavaControlFlowGraph::GetMinDepth: FromOffset:%zi, ToOffset:%zi, Depth:%zi, MinDepth:%zi\n", BlockPtr->FromOffset, BlockPtr->ToOffset, Depth, MinDepth);
        return MinDepth;
    }

    void xJavaControlFlowGraph::UpdateConditionalBranches(xJavaBlock * BlockPtr, xJavaBlock * LeftBlockPtr, xJavaBlock::eType OperatorType, xJavaBlock * SubBlockPtr)
    {
        BlockPtr->Type = OperatorType;
        BlockPtr->ToOffset = SubBlockPtr->ToOffset;
        BlockPtr->NextBlockPtr = SubBlockPtr->NextBlockPtr;
        BlockPtr->BranchBlockPtr = SubBlockPtr->BranchBlockPtr;
        BlockPtr->ConditionBlockPtr = EndBlockPtr;
        BlockPtr->FirstSubBlockPtr = LeftBlockPtr;
        BlockPtr->SecondSubBlockPtr = SubBlockPtr;

        SubBlockPtr->NextBlockPtr->Replace(SubBlockPtr, BlockPtr);
        SubBlockPtr->BranchBlockPtr->Replace(SubBlockPtr, BlockPtr);
    }

    void xJavaControlFlowGraph::UpdateConditionTernaryOperator(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr)
    {
        size_t FromOffset =  NextNextBlockPtr->FromOffset;
        size_t ToOffset   = NextNextBlockPtr->ToOffset;
        xJavaBlock * NewNextBlockPtr = NextNextBlockPtr->NextBlockPtr;
        xJavaBlock * NewBranchBlockPtr = NextNextBlockPtr->BranchBlockPtr;

        if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
        }
        if ((NextNextBlockPtr->Type == xJavaBlock::TYPE_CONDITION) && !NextNextBlockPtr->MustInverseCondition) {
            BlockPtr->InverseCondition();
        }

        auto ConditionBlockPtr = NextNextBlockPtr;
        ConditionBlockPtr->Type = BlockPtr->Type;
        ConditionBlockPtr->FromOffset = BlockPtr->FromOffset;
        ConditionBlockPtr->ToOffset   = BlockPtr->ToOffset;
        ConditionBlockPtr->NextBlockPtr = EndBlockPtr;
        ConditionBlockPtr->BranchBlockPtr = EndBlockPtr;
        ConditionBlockPtr->ConditionBlockPtr = BlockPtr->ConditionBlockPtr;
        ConditionBlockPtr->FirstSubBlockPtr = BlockPtr->FirstSubBlockPtr;
        ConditionBlockPtr->SecondSubBlockPtr = BlockPtr->SecondSubBlockPtr;
        ConditionBlockPtr->Predecessors.clear();

        BlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        BlockPtr->FromOffset = FromOffset;
        BlockPtr->ToOffset = ToOffset;
        BlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        BlockPtr->FirstSubBlockPtr  = BlockPtr->NextBlockPtr;
        BlockPtr->SecondSubBlockPtr = BlockPtr->BranchBlockPtr;
        BlockPtr->NextBlockPtr = NewNextBlockPtr;
        BlockPtr->BranchBlockPtr = NewBranchBlockPtr;
        BlockPtr->FirstSubBlockPtr->NextBlockPtr = EndBlockPtr;
        BlockPtr->SecondSubBlockPtr->NextBlockPtr = EndBlockPtr;

        NewNextBlockPtr->Replace(NextNextBlockPtr, BlockPtr);
        NewBranchBlockPtr->Replace(NextNextBlockPtr, BlockPtr);

        BlockPtr->FirstSubBlockPtr->Predecessors.clear();
        BlockPtr->SecondSubBlockPtr->Predecessors.clear();
    }

    bool xJavaControlFlowGraph::AggregateConditionalBranches(xJavaBlock * BlockPtr)
    {
        auto Change = false;
        auto NextBlockPtr = BlockPtr->NextBlockPtr;
        auto BranchBlockPtr = BlockPtr->BranchBlockPtr;

        if (NextBlockPtr->Type == xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR && NextBlockPtr->Predecessors.size() == 1) {
            auto NextNextBlockPtr = NextBlockPtr->NextBlockPtr;
            if (NextNextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::TYPE_CONDITION)) {
                // TODO
            }
        }

        if (NextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
            // TODO
        }

        if (BranchBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
            // TODO
        }

        if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
            return true;
        }

        return Change;
    }

    bool xJavaControlFlowGraph::ReduceConditionalBranch(xJavaBlock * BlockPtr)
    {
        // TODO
        return true;
    }

    bool xJavaControlFlowGraph::ReduceConditionalBranch(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    {
        while (AggregateConditionalBranches(BlockPtr))
        {}

        assert(BlockPtr->Type & xJavaBlock::GROUP_CONDITION);
        auto ReduceNext = Reduce(BlockPtr->NextBlockPtr, VisitedSet, JsrTargetSet);
        auto ReduceBranch = Reduce(BlockPtr->BranchBlockPtr, VisitedSet, JsrTargetSet);
        if (ReduceNext && ReduceBranch) {
            return ReduceConditionalBranch(BlockPtr);
        }
        return false;
    }

    bool xJavaControlFlowGraph::ReduceSwitchDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    {
        // TODO
        return true;
    }

    bool xJavaControlFlowGraph::ReduceTryDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    {
        // TODO
        return true;
    }

    bool xJavaControlFlowGraph::ReduceJsr(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    {
        // TODO
        return true;
    }

    bool xJavaControlFlowGraph::ReduceLoop(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    {
        // TODO
        return true;
    }

    bool xJavaControlFlowGraph::Reduce(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    {
        if (BlockPtr->Type & xJavaBlock::GROUP_END) {
            return true;
        }
        if (VisitedSet.find(BlockPtr) == VisitedSet.end()) {
            return true;
        }
        VisitedSet.insert(BlockPtr);

        switch (BlockPtr->Type) {
                case xJavaBlock::TYPE_START:
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_IF:
                case xJavaBlock::TYPE_IF_ELSE:
                case xJavaBlock::TYPE_SWITCH:
                case xJavaBlock::TYPE_TRY:
                case xJavaBlock::TYPE_TRY_JSR:
                case xJavaBlock::TYPE_TRY_ECLIPSE:
                case xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR:
                    return Reduce(BlockPtr->NextBlockPtr, VisitedSet, JsrTargetSet);
                case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
                case xJavaBlock::TYPE_CONDITION:
                case xJavaBlock::TYPE_CONDITION_OR:
                case xJavaBlock::TYPE_CONDITION_AND:
                case xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR:
                    return ReduceConditionalBranch(BlockPtr, VisitedSet, JsrTargetSet);
                case xJavaBlock::TYPE_SWITCH_DECLARATION:
                    return ReduceSwitchDeclaration(BlockPtr, VisitedSet, JsrTargetSet);
                case xJavaBlock::TYPE_TRY_DECLARATION:
                    return ReduceTryDeclaration(BlockPtr, VisitedSet, JsrTargetSet);
                case xJavaBlock::TYPE_JSR:
                    return ReduceJsr(BlockPtr, VisitedSet, JsrTargetSet);
                case xJavaBlock::TYPE_LOOP:
                    return ReduceLoop(BlockPtr, VisitedSet, JsrTargetSet);
                default:
                    break;
            }
        return true;
    }

    void xJavaControlFlowGraph::ReduceGraph()
    {
        X_DEBUG_PRINTF("xJavaControlFlowGraph::ReduceGraph: BlockSize=%zi\n", BlockList.size());
        if (Blocks.empty()) {
            return;
        }
        auto StartBlockPtr = Blocks[0];
        if (!StartBlockPtr) {
            return;
        }
        auto VisitedSet = std::set<xJavaBlock*>();
        auto JsrTargetSet = std::set<xJavaBlock*>();
        Reduce(StartBlockPtr, VisitedSet, JsrTargetSet);
    }

}
