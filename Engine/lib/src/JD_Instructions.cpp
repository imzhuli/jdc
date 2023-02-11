#include <jdc/JD_Instructions.hpp>
#include <sstream>
#include <iostream>

using namespace xel;
using namespace std;

namespace jdc
{

    struct xLocalVariable
    {
        std::string Name;
        std::string TypeString;
    };
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
    xOptional<std::string> BuildExpressionLines(std::vector<xLocalVariable> & LocalVariables, xel::xStreamReader & Reader, ssize_t & RemainSize);

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

    std::string BuildCode(const xMethodEx & Method)
    {
        auto & Binary = Method.CodeAttribute.Binary;
        auto LocalVariables = std::vector<xLocalVariable>();
        // push arguments:
        do {
            size_t Total = 0;
            size_t ArgNum = 0;
            size_t VarNum = 0;
            if (!(Method.AccessFlags & ACC_STATIC)) {
                LocalVariables.push_back({ "this", Method.ClassName });
                ++Total;
            }
            while(Total < Method.CodeAttribute.MaxLocals) {
                if (ArgNum < Method.ArgumentTypeStrings.size()) {
                    LocalVariables.push_back(xLocalVariable{ MakeArgumentName(ArgNum), Method.ArgumentTypeStrings[ArgNum] });
                    ++ArgNum;
                } else {
                    LocalVariables.push_back(xLocalVariable{ MakeVariableName(VarNum) });
                    ++VarNum;
                }
                ++Total;
            }
        } while(false);

        cout << "---------> Method: " << Method.Name << endl;
        for (auto & Variable : LocalVariables) {
            cout << "  Local: " << Variable.Name << " " << Variable.TypeString << endl;
        }

        ostringstream ss;
        auto Reader = xStreamReader(Binary.data());
        auto RemainSize = ssize_t(Binary.size());
        auto CodeLine = BuildExpressionLines(LocalVariables, Reader, RemainSize);
        if (CodeLine()) {
            for (auto & Variable : LocalVariables) {
                if (Variable.TypeString.size() && IsLocalVariableName(Variable.Name)) {
                    ss << Variable.TypeString << ' ' << Variable.Name << ';' << endl;
                }
            }
            ss << *CodeLine << endl;
        }

        for (auto & Variable : LocalVariables) {
            cout << "  FixedLocal: " << Variable.Name << " " << Variable.TypeString << endl;
        }

        auto s = ss.str();
        cout << s << endl;
        return s;
    }

    xOptional<std::string> BuildExpressionLines(std::vector<xLocalVariable> & LocalVariables, xel::xStreamReader & Reader, ssize_t & RemainSize)
    {
        assert(RemainSize > 0);

        ostringstream ss;

        auto OpStack = std::vector<std::string>();
        OpStack.reserve(2048);

        do {
            uint8_t OpCode = Reader.R1();
            if ((RemainSize -= 1) < 0) {
                return {};
            }
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

                case OP_ireturn:
                case OP_lreturn:
                case OP_freturn:
                case OP_dreturn: {
                    auto v1 = PopFromStack(OpStack);
                    ss << "return " << v1 << ';' << endl;
                    return { ss.str() };
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

                default: {
                    return {};
                }
            }
        } while(true);

        return { ss.str() };
    }

}