#include <jdc/JD_Instructions.hpp>
#include <xel/String.hpp>
#include <sstream>
#include <iostream>

using namespace xel;
using namespace std;

namespace jdc
{

    #define Check(x) do { if ((RemainSize -= (x)) < 0) { return {}; } } while(false)
    #define CheckAndSkip(x) do { if ((RemainSize -= (x)) < 0) { return {}; } else { Reader.Skip(1); } } while(false)

    std::vector<xEntryMark> BuildEntryMarks(const std::vector<xel::ubyte> & Binary)
    {
        auto Marks = std::vector<xEntryMark>(Binary.size());
        auto Reader = xStreamReader(Binary.data());
        auto RemainSize = ssize_t(Binary.size());

        while(RemainSize-- > 0)
        {
            auto Offset = size_t(Reader.Offset());
            auto OpCode = Reader.R1();

            switch(OpCode) {
                case OP_iload:
                case OP_lload:
                case OP_fload:
                case OP_dload: {
                    CheckAndSkip(1);
                    break;
                }

                case OP_iload_0:
                case OP_lload_0:
                case OP_fload_0:
                case OP_dload_0:
                case OP_iload_1:
                case OP_lload_1:
                case OP_fload_1:
                case OP_dload_1:
                case OP_iload_2:
                case OP_lload_2:
                case OP_fload_2:
                case OP_dload_2:
                case OP_iload_3:
                case OP_lload_3:
                case OP_fload_3:
                case OP_dload_3: {
                    break;
                }

                case OP_istore:{
                    CheckAndSkip(1);
                    break;
                }
                case OP_lstore:{
                    CheckAndSkip(1);
                    break;
                }
                case OP_fstore:{
                    CheckAndSkip(1);
                    break;
                }
                case OP_dstore:{
                    CheckAndSkip(1);
                    break;
                }

                case OP_istore_0:
                case OP_lstore_0:
                case OP_fstore_0:
                case OP_dstore_0:
                case OP_istore_1:
                case OP_lstore_1:
                case OP_fstore_1:
                case OP_dstore_1:
                case OP_istore_2:
                case OP_lstore_2:
                case OP_fstore_2:
                case OP_dstore_2:
                case OP_istore_3:
                case OP_lstore_3:
                case OP_fstore_3:
                case OP_dstore_3: {
                    break;
                }

                case OP_bipush: {
                    CheckAndSkip(1);
                    break;
                }
                case OP_sipush: {
                    CheckAndSkip(2);
                    break;
                }

                case OP_iadd:
                case OP_ladd:
                case OP_fadd:
                case OP_dadd:
                case OP_isub:
                case OP_lsub:
                case OP_fsub:
                case OP_dsub:
                case OP_imul:
                case OP_lmul:
                case OP_fmul:
                case OP_dmul:
                case OP_idiv:
                case OP_ldiv:
                case OP_fdiv:
                case OP_ddiv:
                case OP_i2b:
                case OP_i2c:
                case OP_i2f:
                case OP_i2d:
                case OP_i2s:
                case OP_i2l:
                case OP_l2i:
                case OP_l2f:
                case OP_l2d: {
                    break;
                }

                // consts:
                case OP_iconst_m1:
                case OP_iconst_0:
                case OP_iconst_1:
                case OP_iconst_2:
                case OP_iconst_3:
                case OP_iconst_4:
                case OP_iconst_5:
                case OP_lconst_0:
                case OP_lconst_1:
                case OP_fconst_0:
                case OP_fconst_1:
                case OP_fconst_2:
                case OP_dconst_0:
                case OP_dconst_1:
                    break;

                case OP_ldc:
                    CheckAndSkip(1);
                    break;

                case OP_ldc_w:
                    CheckAndSkip(2);
                    break;

                case OP_ldc2_w:
                    CheckAndSkip(2);

                // if:
                case OP_if_acmpeq:
                case OP_if_acmpne:
                case OP_if_icmpeq:
                case OP_if_icmpne:
                case OP_if_icmplt:
                case OP_if_icmpge:
                case OP_if_icmpgt:
                case OP_if_icmple: {
                    Check(2);
                    uint16_t BlockEndOffset = Reader.R2();
                    uint16_t EntryPointOffset = Offset + BlockEndOffset;
                    auto & Mark = Marks[EntryPointOffset];
                    ++Mark.EntryCount;
                    break;
                }

                case OP_ireturn:
                case OP_lreturn:
                case OP_freturn:
                case OP_dreturn:
                case OP_areturn: {
                    auto & Mark = Marks[Offset];
                    ++Mark.ExitCount;
                }

                // switch:
                case OP_tableswitch: {
                    CheckAndSkip(4);
                    break;
                };

                default: {
                    return {};
                }
            }
        }
        if (!FindEntryPoints(Marks)) {
            return {};
        }
        return Marks;
    }

    bool FindEntryPoints(std::vector<xEntryMark> & Marks)
    {
        auto JumpStack = std::vector<int32_t>();
        JumpStack.reserve(128);


        return true;
    }

}