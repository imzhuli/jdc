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

    void FixLocalVariableType(xLocalVariable & LocalVariable, const char * TypeName)
    {
        if(LocalVariable.TypeString.size()) {
            assert(LocalVariable.TypeString.size() == strlen(TypeName));
            assert(LocalVariable.TypeString.find(TypeName) == 0);
            return;
        }
        if (IsLocalVariableName(LocalVariable.Name)) {
            LocalVariable.TypeString = TypeName;
        }
    }

    std::string PopFromStack(std::vector<std::string> & Stack)
    {
        auto Value = std::move(Stack.back());
        Stack.pop_back();
        return Value;
    }

    const std::string & GetFromStack(const std::vector<std::string> & Stack, ssize_t Index)
    {
        if (Index >= 0) {
            return Stack[Index];
        }
        return Stack[Stack.size() + Index];
    }

    void PushToStack(std::vector<std::string> & Stack, const std::string & Value)
    {
        Stack.push_back(Value);
    }

    xel::xOptional<std::string> BuildExpressionLines(const std::vector<xel::ubyte> & Binary, const std::vector<xConstantItemInfo> & ConstantPool, std::vector<xLocalVariable> & LocalVariables, const std::vector<xCodeLine> & EntryMarks)
    {
        auto ss = ostringstream();
        auto Reader = xStreamReader(Binary.data());
        auto RemainSize = ssize_t(Binary.size());

        auto OpStack = std::vector<std::string>();
        auto BlockEndStack = std::vector<size_t>();
        OpStack.reserve(RemainSize);

        while(RemainSize-- > 0) {
            auto Offset = size_t(Reader.Offset());
            while(BlockEndStack.size() && BlockEndStack.back() == Offset) {
                ss << '}' << endl;
                BlockEndStack.pop_back();
            }

            auto OpCode = Reader.R1();
            switch(OpCode) {
                case OP_iload:
                case OP_lload:
                case OP_fload:
                case OP_dload: {
                    if ((RemainSize -= 1) < 0) {
                        return {};
                    }
                    uint8_t Index = Reader.R1();
                    OpStack.push_back(LocalVariables[Index].Name);
                    break;
                }
                case OP_iload_0:
                case OP_lload_0:
                case OP_fload_0:
                case OP_dload_0:
                    OpStack.push_back(LocalVariables[0].Name);
                    break;
                case OP_iload_1:
                case OP_lload_1:
                case OP_fload_1:
                case OP_dload_1:
                    OpStack.push_back(LocalVariables[1].Name);
                    break;
                case OP_iload_2:
                case OP_lload_2:
                case OP_fload_2:
                case OP_dload_2:
                    OpStack.push_back(LocalVariables[2].Name);
                    break;
                case OP_iload_3:
                case OP_lload_3:
                case OP_fload_3:
                case OP_dload_3:
                    OpStack.push_back(LocalVariables[3].Name);
                    break;

                case OP_istore:{
                    if ((RemainSize -= 1) < 0) {
                        return {};
                    }
                    uint8_t Index = Reader.R1();
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[Index].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[Index], "int");
                    break;
                }
                case OP_lstore:{
                    if ((RemainSize -= 1) < 0) {
                        return {};
                    }
                    uint8_t Index = Reader.R1();
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[Index].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[Index], "long");
                    break;
                }
                case OP_fstore:{
                    if ((RemainSize -= 1) < 0) {
                        return {};
                    }
                    uint8_t Index = Reader.R1();
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[Index].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[Index], "float");
                    break;
                }
                case OP_dstore:{
                    if ((RemainSize -= 1) < 0) {
                        return {};
                    }
                    uint8_t Index = Reader.R1();
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[Index].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[Index], "double");
                    break;
                }
                case OP_istore_0:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[0].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[0], "int");
                    break;
                }
                case OP_lstore_0:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[0].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[0], "long");
                    break;
                }
                case OP_fstore_0:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[0].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[0], "float");
                    break;
                }
                case OP_dstore_0:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[0].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[0], "double");
                    break;
                }
                case OP_istore_1:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[1].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[1], "int");
                    break;
                }
                case OP_lstore_1:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[1].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[1], "long");
                    break;
                }
                case OP_fstore_1:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[1].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[1], "float");
                    break;
                }
                case OP_dstore_1:  {
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[1].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[1], "double");
                    break;
                }
                case OP_istore_2: {
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[2].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[2], "int");
                    break;
                }
                case OP_lstore_2: {
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[2].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[2], "long");
                    break;
                }
                case OP_fstore_2: {
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[2].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[2], "float");
                    break;
                }
                case OP_dstore_2: {
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[2].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[2], "double");
                    break;
                }
                case OP_istore_3:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[3].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[3], "int");
                    break;
                }
                case OP_lstore_3:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[3].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[3], "long");
                    break;
                }
                case OP_fstore_3:{
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[3].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[3], "float");
                    break;
                }
                case OP_dstore_3: {
                    auto v1 = PopFromStack(OpStack);
                    ss << LocalVariables[3].Name << " = " << v1 << ';'  << endl;
                    FixLocalVariableType(LocalVariables[3], "double");
                    break;
                }

                case OP_bipush: {
                    if ((RemainSize -= 1) < 0) {
                        return {};
                    }
                    int8_t Byte = Reader.R1();
                    PushToStack(OpStack, std::to_string(Byte));
                    break;
                }
                case OP_sipush: {
                    if ((RemainSize -= 2) < 0) {
                        return {};
                    }
                    int16_t Short = Reader.R2();
                    PushToStack(OpStack, std::to_string(Short));
                    break;
                }

                case OP_iadd:
                case OP_ladd:
                case OP_fadd:
                case OP_dadd: {
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(" + v1 + ") + (" + v2 + ")");
                    break;
                }
                case OP_isub:
                case OP_lsub:
                case OP_fsub:
                case OP_dsub: {
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(" + v1 + ") - (" + v2 + ")");
                    break;
                }
                case OP_imul:
                case OP_lmul:
                case OP_fmul:
                case OP_dmul: {
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(" + v1 + ") * (" + v2 + ")");
                    break;
                }
                case OP_idiv:
                case OP_ldiv:
                case OP_fdiv:
                case OP_ddiv: {
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(" + v1 + ") / (" + v2 + ")");
                    break;
                }

                case OP_i2b: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(bool)(" + v1 + ")");
                    break;
                }
                case OP_i2c: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(char)(" + v1 + ")");
                    break;
                }
                case OP_i2f: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(float)(" + v1 + ")");
                    break;
                }
                case OP_i2d: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(double)(" + v1 + ")");
                    break;
                }
                case OP_i2s: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(short)(" + v1 + ")");
                    break;
                }
                case OP_i2l: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(long)(" + v1 + ")");
                    break;
                }
                case OP_l2i: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(int)(" + v1 + ")");
                    break;
                }
                case OP_l2f: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(float)(" + v1 + ")");
                    break;
                }
                case OP_l2d: {
                    auto v1 = PopFromStack(OpStack);
                    PushToStack(OpStack, "(double)(" + v1 + ")");
                    break;
                }

                // consts:
                case OP_iconst_m1:
                    PushToStack(OpStack, "-1");
                    break;
                case OP_iconst_0:
                    PushToStack(OpStack, "0");
                    break;
                case OP_iconst_1:
                    PushToStack(OpStack, "1");
                    break;
                case OP_iconst_2:
                    PushToStack(OpStack, "2");
                    break;
                case OP_iconst_3:
                    PushToStack(OpStack, "3");
                    break;
                case OP_iconst_4:
                    PushToStack(OpStack, "4");
                    break;
                case OP_iconst_5:
                    PushToStack(OpStack, "5");
                    break;

                case OP_lconst_0:
                    PushToStack(OpStack, "0");
                    break;
                case OP_lconst_1:
                    PushToStack(OpStack, "1");
                    break;

                case OP_fconst_0:
                    PushToStack(OpStack, "0.f");
                    break;
                case OP_fconst_1:
                    PushToStack(OpStack, "1.f");
                    break;
                case OP_fconst_2:
                    PushToStack(OpStack, "2.f");
                    break;

                case OP_dconst_0:
                    PushToStack(OpStack, "0.0");
                    break;
                case OP_dconst_1:
                    PushToStack(OpStack, "1.0");
                    break;

                case OP_ldc: {
                    Check(1);
                    auto Index = Reader.R1();
                    PushToStack(OpStack, ConstantValueString(ConstantPool, Index));
                    break;
                }
                case OP_ldc_w: {
                    Check(2);
                    auto Index = Reader.R2();
                    PushToStack(OpStack, ConstantValueString(ConstantPool, Index));
                    break;
                }

                case OP_ldc2_w:{ // TODO:
                    Check(2);
                    auto Index = Reader.R2();
                    PushToStack(OpStack, ConstantValueString(ConstantPool, Index));
                    break;
                }

                // if:
                case OP_if_acmpeq:
                case OP_if_acmpne:
                    assert("NotImplemented");
                    return {};

                case OP_if_icmpeq:{ // ==
                    if ((RemainSize -= 2) < 0) {
                        return {};
                    }
                    uint16_t BlockEndOffset = Reader.R2();
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    ss << "if ((" << v1 << ") != (" << v2 << ")) {" << endl;
                    BlockEndStack.push_back(Offset + BlockEndOffset);
                    break;
                }
                case OP_if_icmpne:{ // !=
                    if ((RemainSize -= 2) < 0) {
                        return {};
                    }
                    uint16_t BlockEndOffset = Reader.R2();
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    ss << "if ((" << v1 << ") == (" << v2 << ")) {" << endl;
                    BlockEndStack.push_back(Offset + BlockEndOffset);
                    break;
                }
                case OP_if_icmplt:{ // <
                    if ((RemainSize -= 2) < 0) {
                        return {};
                    }
                    uint16_t BlockEndOffset = Reader.R2();
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    ss << "if ((" << v1 << ") >= (" << v2 << ")) {" << endl;
                    BlockEndStack.push_back(Offset + BlockEndOffset);
                    break;
                }
                case OP_if_icmpge:{ // >=
                    if ((RemainSize -= 2) < 0) {
                        return {};
                    }
                    uint16_t BlockEndOffset = Reader.R2();
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    ss << "if ((" << v1 << ") < (" << v2 << ")) {" << endl;
                    BlockEndStack.push_back(Offset + BlockEndOffset);
                    break;
                }
                case OP_if_icmpgt: { // >
                    if ((RemainSize -= 2) < 0) {
                        return {};
                    }
                    uint16_t BlockEndOffset = Reader.R2();
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    ss << "if ((" << v1 << ") <= (" << v2 << ")) {" << endl;
                    BlockEndStack.push_back(Offset + BlockEndOffset);
                    break;
                }
                case OP_if_icmple: { // <=
                    if ((RemainSize -= 2) < 0) {
                        return {};
                    }
                    uint16_t BlockEndOffset = Reader.R2();
                    auto v2 = PopFromStack(OpStack);
                    auto v1 = PopFromStack(OpStack);
                    ss << "if ((" << v1 << ") > (" << v2 << ")) {" << endl;
                    BlockEndStack.push_back(Offset + BlockEndOffset);
                    break;
                }

                case OP_ireturn:
                case OP_lreturn:
                case OP_freturn:
                case OP_dreturn: {
                    auto v1 = PopFromStack(OpStack);
                    ss << "return " << v1 << ';' << endl;
                    break;
                }
                case OP_areturn: {
                    auto v1 = PopFromStack(OpStack);
                    ss << "return " << v1 << ';' << endl;
                    break;
                }

                // switch:
                case OP_tableswitch: {
                    size_t Align = Offset & 0x03;
                    if (Align) {
                        Align = 4 - Align;
                        if ((RemainSize -= Align) < 0) {
                            return {};
                        }
                        Reader.Skip(Align);
                    }
                    if ((RemainSize -= 4) < 0) { // default:
                        return {};
                    }
                    uint32_t DefaultJumpOffset = Reader.R4();


                    (void)DefaultJumpOffset;
                    break;
                };

                default: {
                    return {};
                }
            }
        }

        return { ss.str() };
    }

}