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

    enum eCodeType : uint8_t
    {
        CT_UNKNOWN       = 0,
        CT_GOTO          = 'g',  // 'g'
        CT_TERNARY_GOTO  = 'G',  // 'G'
        CT_THROW         = 't',  // 't'
        CT_RETURN        = 'r',  // 'r'
        CT_CONDITIONAL   = 'c',  // 'c'
        CT_SWITCH        = 's',  // 's'
        CT_JSR           = 'j',  // 'j'
        CT_RET           = 'R',  // 'R'
        CT_RETURN_VALUE  = 'v',  // 'v'
        CT_TRY           = 'T',  // 'T'

        // default:
        // CT_STATEMENT     = ' '// TYPE_STATEMENTS
    };

    #define CODE_TYPE_TO_STRING(x) case (x) : do { return #x; } while(false)
    const char * ToCString(eCodeType CT)
    {
        switch(CT) {
            CODE_TYPE_TO_STRING(CT_UNKNOWN);
            CODE_TYPE_TO_STRING(CT_GOTO);
            CODE_TYPE_TO_STRING(CT_TERNARY_GOTO);
            CODE_TYPE_TO_STRING(CT_THROW);
            CODE_TYPE_TO_STRING(CT_RETURN);
            CODE_TYPE_TO_STRING(CT_CONDITIONAL);
            CODE_TYPE_TO_STRING(CT_SWITCH);
            CODE_TYPE_TO_STRING(CT_JSR);
            CODE_TYPE_TO_STRING(CT_RET);
            CODE_TYPE_TO_STRING(CT_RETURN_VALUE);
            CODE_TYPE_TO_STRING(CT_TRY);
            default: break;
        }
        return "CT_STATEMENT";
    }

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

        auto CodeAttributePtr = GetCodeAttribute();
        auto & CodeBinary = CodeAttributePtr->CodeBinary;
        size_t CodeLength = CodeBinary.size();
        auto & ExceptionTable = CodeAttributePtr->ExceptionTable;

        auto BlockMap           = std::vector<xJavaBlock*>(CodeLength);
        auto CodeTypes          = std::vector<eCodeType>(CodeLength);
        auto NextOffsets        = std::vector<size_t>(CodeLength);
        auto BranchOffsets      = std::vector<size_t>(CodeLength);;
        auto SwitchValueTable   = std::vector<std::vector<int32_t>>(CodeLength);
        auto SwitchOffsetTable  = std::vector<std::vector<size_t>>(CodeLength);

        auto MARK = EndBlockPtr;
        BlockMap[0] = MARK;

        /**
         * @brief Build CodeTypes & BlockMap
         *
         */

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
                        BlockMap[Offset + 1] = MARK;
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
                        BlockMap[LastStatementOffset + 1] = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Offset += 2;
                    CodeTypes[Offset] = CodeType;
                    BlockMap[BranchOffset] = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockMap[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
                case OP_JSR: {
                    if (LastStatementOffset != size_t(-1)) {
                        BlockMap[LastStatementOffset + 1] = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    CodeTypes[Offset] = CT_JSR;
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Offset += 2;
                    CodeTypes[Offset] = CT_JSR;
                    BlockMap[BranchOffset] = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockMap[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
                case OP_IFEQ:      case OP_IFNE:      case OP_IFLT:      case OP_IFGE:      case OP_IFGT:      case OP_IFLE:
                case OP_IF_ICMPEQ: case OP_IF_ICMPNE: case OP_IF_ICMPLT: case OP_IF_ICMPGE: case OP_IF_ICMPGT: case OP_IF_ICMPLE: case OP_IF_ACMPEQ: case OP_IF_ACMPNE:
                case OP_IFNULL:    case OP_IFNONNULL: {
                    if (LastStatementOffset != size_t(-1)) {
                        BlockMap[LastStatementOffset + 1] = MARK;
                    }
                    // The target of a conditional or an unconditional goto/jump instruction is a leader
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + (int16_t)Reader.R2();
                    Offset += 2;
                    CodeTypes[Offset] = CT_CONDITIONAL;
                    BlockMap[BranchOffset] = MARK;
                    BranchOffsets[Offset] = BranchOffset;
                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockMap[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }

                case OP_TABLESWITCH: {
                    // skip padding:
                    Reader.Offset((Offset + 4) & 0x00FFFC);
                    size_t DefaultOffset = Offset + Reader.R4();
                    BlockMap[DefaultOffset] = MARK;

                    auto Low = (int32_t)Reader.R4();
                    auto High = (int32_t)Reader.R4();
                    auto Values  = std::vector<int32_t>(High - Low + 2);
                    auto Offsets = std::vector<size_t>(High - Low + 2);

                    Offsets[0] = DefaultOffset;

                    for (int32_t I = 1, Len = High - Low + 2; I < Len; I++) {
                        Values[I] = Low + I - 1;
                        size_t BranchOffset = Offsets[I] = Offset + Reader.R4();
                        BlockMap[BranchOffset] = MARK;
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
                        BlockMap[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;

                case OP_RETURN:
                    if (LastStatementOffset != (size_t)-1) {
                        BlockMap[LastStatementOffset + 1] = MARK;
                    }
                    CodeTypes[Offset] = CT_RETURN;
                    if (Offset + 1 < CodeLength) {
                        BlockMap[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;

                case OP_ATHROW:
                    CodeTypes[Offset] = CT_THROW;
                    if (Offset + 1 < CodeLength) {
                        BlockMap[Offset + 1] = MARK;
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
                                BlockMap[Offset + 1] = MARK;
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

                    BlockMap[BranchOffset] = MARK;
                    CodeTypes[Offset] = CodeType;
                    BranchOffsets[Offset] = BranchOffset;

                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockMap[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }

                case OP_JSR_W: {
                    if (LastStatementOffset != (size_t)-1) {
                        BlockMap[LastStatementOffset + 1] = MARK;
                    }
                    Reader.Offset(Offset + 1);
                    size_t BranchOffset = Offset + Reader.R4();
                    Offset += 4;

                    BlockMap[BranchOffset] = MARK;
                    CodeTypes[Offset] = CT_JSR;
                    BranchOffsets[Offset] = BranchOffset;

                    // The instruction that immediately follows a conditional or an unconditional goto/jump instruction is a leader
                    if (Offset + 1 < CodeLength) {
                        BlockMap[Offset + 1] = MARK;
                    }
                    LastStatementOffset = Offset;
                    break;
                }
            } // end of switch(opcode)
        } // end of for
        NextOffsets[LastOffset] = CodeLength;

        /***
         * @brief Ignore linetables
        */

        /***
         * @brief Processing Exception table
         *
         */
        if (ExceptionTable.size()) {
            for (auto & Mark : ExceptionTable) {
                BlockMap[Mark.StartPC] = MARK;
                BlockMap[Mark.HandlerPC] = MARK;
            }
        }

        /**
         * @brief Create basic blocks
         *
         */
        LastOffset = 0;
        auto StartBlockPtr = NewBlock(xJavaBlock::TYPE_START);
        for (size_t Offset = NextOffsets[0]; Offset < CodeLength; Offset = NextOffsets[Offset]) {
            if (BlockMap[Offset]) { // replace marked block with true one.
                BlockMap[LastOffset] = NewBlock(xJavaBlock::TYPE_DELETED, LastOffset, Offset);
                LastOffset = Offset;
            }
        }
        BlockMap[LastOffset] = NewBlock(xJavaBlock::TYPE_DELETED, LastOffset, CodeLength);

        /**
         * @brief reduce to simple Blocks and set real types
         *
         */

        // set block types:
        Blocks.reserve(BlockList.size());
        auto SuccessBlockPtr = BlockList[1].get();
        StartBlockPtr->NextBlockPtr = SuccessBlockPtr;
        SuccessBlockPtr->Predecessors.insert(StartBlockPtr);

        auto BlockListLength = BlockList.size();
        for (size_t I = 1; I < BlockListLength; ++I) {
            auto BlockPtr = BlockList[I].get();
            size_t LastInstructionOffset = BlockPtr->ToOffset - 1;

            switch(CodeTypes[LastInstructionOffset]) {
                case CT_GOTO:
                    BlockPtr->Type = xJavaBlock::TYPE_GOTO;
                    SuccessBlockPtr = BlockMap[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    break;
                case CT_TERNARY_GOTO:
                    BlockPtr->Type = xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR;
                    SuccessBlockPtr = BlockMap[BranchOffsets[LastInstructionOffset]];
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
                    SuccessBlockPtr = BlockMap[BlockPtr->ToOffset];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    SuccessBlockPtr = BlockMap[BranchOffsets[LastInstructionOffset]];
                    BlockPtr->BranchBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    break;
                case CT_SWITCH: {
                    BlockPtr->Type = xJavaBlock::TYPE_SWITCH_DECLARATION;
                    auto & Values = SwitchValueTable[LastInstructionOffset];
                    auto & Offsets = SwitchOffsetTable[LastInstructionOffset];
                    auto & SwitchCases = BlockPtr->SwitchCases;

                    size_t DefaultCaseOffset = Offsets[0];
                    auto CaseBlockPtr = BlockMap[DefaultCaseOffset];
                    SwitchCases.emplace_back(BlockPtr);
                    CaseBlockPtr->Predecessors.insert(BlockPtr);

                    for (size_t Index = 1; Index < Offsets.size(); ++Index) {
                        size_t CaseOffset = Offsets[Index];
                        if (CaseOffset != DefaultCaseOffset) { // merge case shared with default
                            CaseBlockPtr = BlockMap[CaseOffset];
                            SwitchCases.emplace_back(Values[Index], CaseBlockPtr);
                            CaseBlockPtr->Predecessors.insert(BlockPtr);
                        }
                    }
                    break;
                }
                case CT_JSR:
                    BlockPtr->Type = xJavaBlock::TYPE_JSR;
                    SuccessBlockPtr = BlockMap[BlockPtr->ToOffset];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    SuccessBlockPtr = BlockMap[BranchOffsets[LastInstructionOffset]];
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
                    SuccessBlockPtr = BlockMap[BlockPtr->ToOffset];
                    BlockPtr->NextBlockPtr = SuccessBlockPtr;
                    SuccessBlockPtr->Predecessors.insert(BlockPtr);
                    Blocks.push_back(BlockPtr);
                    break;
            }
        }

        // --- Create try-catch-finally basic blocks --- //
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
                    continue;
                }
                if (HandlePCMarks[HandlerPC] == CT_TRY && StartPC > BlockMap[HandlePCToStartPC[HandlerPC]]->FromOffset) {
                    X_DEBUG_BREAKPOINT();
                    continue;
                }

                auto & TryCatchFinalBlockPtr = Cache[CE];
                if (!TryCatchFinalBlockPtr) {
                    // Insert a new 'try-catch-finally' basic block
                    auto StartBlockPtr = BlockMap[StartPC];
                    NewBlock(xJavaBlock::TYPE_TRY_DECLARATION, StartPC, EndPC);
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
                    BlockMap[StartPC] = TryCatchFinalBlockPtr;
                    Cache[CE] = TryCatchFinalBlockPtr;
                }

                if (CE.CatchType) {
                    auto & UnfixedTypeBinaryName = _JavaClassPtr->ClassInfo.GetConstantClassBinaryName(CE.CatchType);
                    CE.FixedExceptionClassBinaryName = _JavaClassPtr->JavaSpacePtr->GetFixedClassBinaryName(UnfixedTypeBinaryName);

                    X_DEBUG_PRINTF("ExceptionClassBinaryName: %s\n", CE.FixedExceptionClassBinaryName.c_str());
                }

                auto HandlerBlockPtr = BlockMap[HandlerPC];
                auto Handler = xJavaExceptionHandler{ CE.FixedExceptionClassBinaryName, HandlerBlockPtr };
                TryCatchFinalBlockPtr->AddExceptionHandler(Handler);

                HandlerBlockPtr->Predecessors.insert(TryCatchFinalBlockPtr);
                HandlePCToStartPC[HandlerPC] = StartPC;
                HandlePCMarks[HandlerPC] = CT_THROW;
            }
        }

        /* --- Recheck TYPE_GOTO_IN_TERNARY_OPERATOR --- */
        for (auto BlockPtr : Blocks) {
            assert(BlockPtr);
            auto NextBlockPtr = BlockPtr->NextBlockPtr;
            auto PredecessorsPtr = static_cast<std::set<xJavaBlock *>*>(nullptr);

            if (BlockPtr->Type != xJavaBlock::TYPE_STATEMENTS) {
                continue;
            }
            if (NextBlockPtr->Predecessors.size() != 1) {
                continue;
            }

            if (NextBlockPtr->Type == xJavaBlock::TYPE_GOTO && BlockPtr->EvalStackDepth() > 0) {
                // Transform STATEMENTS and GOTO to GOTO_IN_TERNARY_OPERATOR
                BlockPtr->Type = xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR;
                BlockPtr->ToOffset = NextBlockPtr->ToOffset;

                BlockPtr->NextBlockPtr = NextBlockPtr->NextBlockPtr;
                PredecessorsPtr = & NextBlockPtr->NextBlockPtr->Predecessors;
                PredecessorsPtr->erase(NextBlockPtr);
                PredecessorsPtr->insert(BlockPtr);

                NextBlockPtr->Type = xJavaBlock::TYPE_DELETED;
            }
            else if (NextBlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH && BlockPtr->EvalStackDepth() > 0) {
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

    // void xJavaControlFlowGraph::UpdateConditionalBranches(xJavaBlock * BlockPtr, xJavaBlock * LeftBlockPtr, xJavaBlock::eType OperatorType, xJavaBlock * SubBlockPtr)
    // {
        // BlockPtr->Type = OperatorType;
        // BlockPtr->ToOffset = SubBlockPtr->ToOffset;
        // BlockPtr->NextBlockPtr = SubBlockPtr->NextBlockPtr;
        // BlockPtr->BranchBlockPtr = SubBlockPtr->BranchBlockPtr;
        // BlockPtr->ConditionBlockPtr = EndBlockPtr;
        // BlockPtr->FirstSubBlockPtr = LeftBlockPtr;
        // BlockPtr->SecondSubBlockPtr = SubBlockPtr;

        // SubBlockPtr->NextBlockPtr->Replace(SubBlockPtr, BlockPtr);
        // SubBlockPtr->BranchBlockPtr->Replace(SubBlockPtr, BlockPtr);
    // }

    // void xJavaControlFlowGraph::UpdateConditionTernaryOperator(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr)
    // {
        // size_t FromOffset =  NextNextBlockPtr->FromOffset;
        // size_t ToOffset   = NextNextBlockPtr->ToOffset;
        // xJavaBlock * NewNextBlockPtr = NextNextBlockPtr->NextBlockPtr;
        // xJavaBlock * NewBranchBlockPtr = NextNextBlockPtr->BranchBlockPtr;

        // if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
        //     BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
        // }
        // if ((NextNextBlockPtr->Type == xJavaBlock::TYPE_CONDITION) && !NextNextBlockPtr->MustInverseCondition) {
        //     BlockPtr->InverseCondition();
        // }

        // auto ConditionBlockPtr = NextNextBlockPtr;
        // ConditionBlockPtr->Type = BlockPtr->Type;
        // ConditionBlockPtr->FromOffset = BlockPtr->FromOffset;
        // ConditionBlockPtr->ToOffset   = BlockPtr->ToOffset;
        // ConditionBlockPtr->NextBlockPtr = EndBlockPtr;
        // ConditionBlockPtr->BranchBlockPtr = EndBlockPtr;
        // ConditionBlockPtr->ConditionBlockPtr = BlockPtr->ConditionBlockPtr;
        // ConditionBlockPtr->FirstSubBlockPtr = BlockPtr->FirstSubBlockPtr;
        // ConditionBlockPtr->SecondSubBlockPtr = BlockPtr->SecondSubBlockPtr;
        // ConditionBlockPtr->Predecessors.clear();

        // BlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        // BlockPtr->FromOffset = FromOffset;
        // BlockPtr->ToOffset = ToOffset;
        // BlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        // BlockPtr->FirstSubBlockPtr  = BlockPtr->NextBlockPtr;
        // BlockPtr->SecondSubBlockPtr = BlockPtr->BranchBlockPtr;
        // BlockPtr->NextBlockPtr = NewNextBlockPtr;
        // BlockPtr->BranchBlockPtr = NewBranchBlockPtr;
        // BlockPtr->FirstSubBlockPtr->NextBlockPtr = EndBlockPtr;
        // BlockPtr->SecondSubBlockPtr->NextBlockPtr = EndBlockPtr;

        // NewNextBlockPtr->Replace(NextNextBlockPtr, BlockPtr);
        // NewBranchBlockPtr->Replace(NextNextBlockPtr, BlockPtr);

        // BlockPtr->FirstSubBlockPtr->Predecessors.clear();
        // BlockPtr->SecondSubBlockPtr->Predecessors.clear();
    // }

    // void xJavaControlFlowGraph::UpdateCondition(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr, xJavaBlock * NextNextNextNextBlockPtr)
    // {
        // size_t FromOffset =  NextNextNextNextBlockPtr->FromOffset;
        // size_t ToOffset = NextNextNextNextBlockPtr->ToOffset;
        // xJavaBlock * NextBlockPtr = NextNextNextNextBlockPtr->NextBlockPtr;
        // xJavaBlock * BranchBlockPtr = NextNextNextNextBlockPtr->BranchBlockPtr;

        // xJavaBlock * ConditionBlockPtr = BlockPtr->GetControlFlowGraph()->newBasicBlock(BlockPtr);
        // ConditionBlockPtr->Type = TYPE_CONDITION;

        // BlockPtr->NextBlockPtr->NextBlockPtr = EndBlockPtr;
        // BlockPtr->NextBlockPtr->Predecessors.clear();
        // BlockPtr->BranchBlockPtr->NextBlockPtr = EndBlockPtr;
        // BlockPtr->BranchBlockPtr->Predecessors.clear();

        // NextNextNextNextBlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        // NextNextNextNextBlockPtr->FromOffset = ConditionBlockPtr->ToOffset;
        // NextNextNextNextBlockPtr->ToOffset = ConditionBlockPtr->ToOffset;
        // NextNextNextNextBlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        // NextNextNextNextBlockPtr->FirstSubBlockPtr = BlockPtr->NextBlockPtr;
        // NextNextNextNextBlockPtr->SecondSubBlockPtr = BlockPtr->BranchBlockPtr;
        // NextNextNextNextBlockPtr->NextBlockPtr = EndBlockPtr;
        // NextNextNextNextBlockPtr->BranchBlockPtr = EndBlockPtr;
        // ConditionBlockPtr->NextBlockPtr = EndBlockPtr;
        // ConditionBlockPtr->BranchBlockPtr = EndBlockPtr;

        // ConditionBlockPtr = NextNextBlockPtr->getControlFlowGraph()->newBasicBlock(NextNextBlockPtr);
        // ConditionBlockPtr->Type = xJavaBlock::TYPE_CONDITION;

        // NextNextBlockPtr->NextBlockPtr->NextBlockPtr = EndBlockPtr;
        // NextNextBlockPtr->NextBlockPtr->Predecessors.clear();
        // NextNextBlockPtr->BranchBlockPtr->NextBlockPtr =EndBlockPtr;
        // NextNextBlockPtr->BranchBlockPtr->Predecessors.clear();

        // NextNextBlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        // NextNextBlockPtr->FromOffset = ConditionBlockPtr->ToOffset;
        // NextNextBlockPtr->ToOffset = ConditionBlockPtr->ToOffset;
        // NextNextBlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        // NextNextBlockPtr->FirstSubBlockPtr = NextNextBlockPtr->NextBlockPtr;
        // NextNextBlockPtr->SecondSubBlockPtr = NextNextBlockPtr->BranchBlockPtr;
        // NextNextBlockPtr->NextBlockPtr = EndBlockPtr;
        // NextNextBlockPtr->BranchBlockPtr = EndBlockPtr;
        // ConditionBlockPtr->NextBlockPtr = EndBlockPtr;
        // ConditionBlockPtr->BranchBlockPtr = EndBlockPtr;

        // BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
        // BlockPtr->FromOffset = FromOffset;
        // BlockPtr->ToOffset = ToOffset;
        // BlockPtr->FirstSubBlockPtr = NextNextNextNextBlockPtr;
        // BlockPtr->SecondSubBlockPtr = NextNextBlockPtr;
        // BlockPtr->NextBlockPtr = NextBlockPtr;
        // BlockPtr->BranchBlockPtr = BranchBlockPtr;

        // NextBlockPtr->Replace(NextNextNextNextBlockPtr, BlockPtr);
        // BranchBlockPtr->Replace(NextNextNextNextBlockPtr, BlockPtr);
    // }

    // bool xJavaControlFlowGraph::AggregateConditionalBranches(xJavaBlock * BlockPtr)
    // {
    //     auto Change = false;
    //     auto NextBlockPtr = BlockPtr->NextBlockPtr;
    //     auto BranchBlockPtr = BlockPtr->BranchBlockPtr;

    //     if (NextBlockPtr->Type == xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR && NextBlockPtr->Predecessors.size() == 1) {
    //         auto NextNextBlockPtr = NextBlockPtr->NextBlockPtr;

    //         if (NextNextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::TYPE_CONDITION)) {
    //             if ((BranchBlockPtr->Type & (xJavaBlock::TYPE_STATEMENTS | xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR))
    //              && (NextNextBlockPtr == BranchBlockPtr->NextBlockPtr)
    //              && (BranchBlockPtr->Predecessors.size() == 1)
    //              && (NextNextBlockPtr->Predecessors.size() == 2)) {
    //                 if (GetMinDepth(NextNextBlockPtr) == -1) {
    //                     UpdateConditionTernaryOperator(BlockPtr, NextNextBlockPtr);
    //                     return true;
    //                 }
    //                 auto NextNextNextBlockPtr = NextNextBlockPtr->NextBlockPtr;
    //                 auto NextNextBranchBlockPtr = NextNextBlockPtr->BranchBlockPtr;

    //                 if (NextNextNextBlockPtr->Type == xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR && NextNextNextBlockPtr->Predecessors.size() == 1) {
    //                     auto NextNextNextNextBlockPtr = NextNextNextBlockPtr->NextBlockPtr;

    //                     if (NextNextNextNextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::TYPE_CONDITION)) {
    //                         if ((NextNextBranchBlockPtr->Type & (xJavaBlock::TYPE_STATEMENTS | xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR))
    //                          && (NextNextNextNextBlockPtr == NextNextBranchBlockPtr->NextBlockPtr)
    //                          && (NextNextBlockPtr->Predecessors.size() == 1)
    //                          && (NextNextNextNextBlockPtr->Predecessors.size() == 2)) {
    //                             if (GetMinDepth(NextNextNextNextBlockPtr) == -2) {
    //                                 UpdateCondition(BlockPtr, NextNextBlockPtr, NextNextNextNextBlockPtr);
    //                                 return true;
    //                             }
    //                          }
    //                     }
    //                 }
    //             }

    //             //TODO

    //         }
    //     }

    //     if (NextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
    //         // TODO
    //     }

    //     if (BranchBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
    //         // TODO
    //     }

    //     if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
    //         BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
    //         return true;
    //     }

    //     return Change;
    // }

    // bool xJavaControlFlowGraph::ReduceConditionalBranch(xJavaBlock * BlockPtr)
    // {
    //     // TODO
    //     return true;
    // }

    // bool xJavaControlFlowGraph::ReduceConditionalBranch(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    // {
    //     while (AggregateConditionalBranches(BlockPtr))
    //     {}

    //     assert(BlockPtr->Type & xJavaBlock::GROUP_CONDITION);
    //     auto ReduceNext = Reduce(BlockPtr->NextBlockPtr, VisitedSet, JsrTargetSet);
    //     auto ReduceBranch = Reduce(BlockPtr->BranchBlockPtr, VisitedSet, JsrTargetSet);
    //     if (ReduceNext && ReduceBranch) {
    //         return ReduceConditionalBranch(BlockPtr);
    //     }
    //     return false;
    // }

    // bool xJavaControlFlowGraph::ReduceSwitchDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    // {
    //     // TODO
    //     return true;
    // }

    // bool xJavaControlFlowGraph::ReduceTryDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    // {
    //     // TODO
    //     return true;
    // }

    // bool xJavaControlFlowGraph::ReduceJsr(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    // {
    //     // TODO
    //     return true;
    // }

    // bool xJavaControlFlowGraph::ReduceLoop(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    // {
    //     // TODO
    //     return true;
    // }

    // bool xJavaControlFlowGraph::Reduce(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet)
    // {
    //     if (BlockPtr->Type & xJavaBlock::GROUP_END) {
    //         return true;
    //     }
    //     if (VisitedSet.find(BlockPtr) == VisitedSet.end()) {
    //         return true;
    //     }
    //     VisitedSet.insert(BlockPtr);

    //     switch (BlockPtr->Type) {
    //             case xJavaBlock::TYPE_START:
    //             case xJavaBlock::TYPE_STATEMENTS:
    //             case xJavaBlock::TYPE_IF:
    //             case xJavaBlock::TYPE_IF_ELSE:
    //             case xJavaBlock::TYPE_SWITCH:
    //             case xJavaBlock::TYPE_TRY:
    //             case xJavaBlock::TYPE_TRY_JSR:
    //             case xJavaBlock::TYPE_TRY_ECLIPSE:
    //             case xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR:
    //                 return Reduce(BlockPtr->NextBlockPtr, VisitedSet, JsrTargetSet);
    //             case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
    //             case xJavaBlock::TYPE_CONDITION:
    //             case xJavaBlock::TYPE_CONDITION_OR:
    //             case xJavaBlock::TYPE_CONDITION_AND:
    //             case xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR:
    //                 return ReduceConditionalBranch(BlockPtr, VisitedSet, JsrTargetSet);
    //             case xJavaBlock::TYPE_SWITCH_DECLARATION:
    //                 return ReduceSwitchDeclaration(BlockPtr, VisitedSet, JsrTargetSet);
    //             case xJavaBlock::TYPE_TRY_DECLARATION:
    //                 return ReduceTryDeclaration(BlockPtr, VisitedSet, JsrTargetSet);
    //             case xJavaBlock::TYPE_JSR:
    //                 return ReduceJsr(BlockPtr, VisitedSet, JsrTargetSet);
    //             case xJavaBlock::TYPE_LOOP:
    //                 return ReduceLoop(BlockPtr, VisitedSet, JsrTargetSet);
    //             default:
    //                 break;
    //         }
    //     return true;
    // }

    // void xJavaControlFlowGraph::ReduceGraph()
    // {
    //     X_DEBUG_PRINTF("xJavaControlFlowGraph::ReduceGraph: BlockSize=%zi\n", BlockList.size());
    //     if (Blocks.empty()) {
    //         return;
    //     }
    //     auto StartBlockPtr = Blocks[0];
    //     if (!StartBlockPtr) {
    //         return;
    //     }
    //     auto VisitedSet = std::set<xJavaBlock*>();
    //     auto JsrTargetSet = std::set<xJavaBlock*>();
    //     Reduce(StartBlockPtr, VisitedSet, JsrTargetSet);
    // }

}
