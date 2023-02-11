#include <jdc/JD_Class.hpp>
#include <jdc/JD_ClassEx.hpp>
#include <sstream>
#include <iostream>

using namespace xel;
using namespace std;

namespace jdc
{

    xOptional<std::string> BuildExpressionLine(xel::xStreamReader & Reader, ssize_t & RemainSize);

    std::string PopOperandFromStack(std::vector<std::string> & Stack)
    {
        auto Value = std::move(Stack.back());
        Stack.pop_back();
        return Value;
    }

    const std::string & GetOperandFromStack(const std::vector<std::string> & Stack, ssize_t Index)
    {
        if (Index >= 0) {
            return Stack[Index];
        }
        return Stack[Stack.size() + Index];
    }

    std::string BuildCode(const xMethodEx & Method)
    {
        std::string Code;

        auto & Binary = Method.CodeAttribute.Binary;
        auto StackVariableNames = std::vector<std::string>() ;
        // push arguments:
        for (size_t i = 0 ; i < Method.CodeAttribute.MaxLocals; ++i) {
            if (i < Method.ArgumentSize) {
                StackVariableNames.push_back(MakeArgumentName(i));
            } else {
                StackVariableNames.push_back(MakeVariableName(i));
            }
        }

        cout << "---------> Method: " << Method.Name << endl;
        for (auto & VariableName : StackVariableNames) {
            cout << "  Local: " << VariableName << endl;
        }

        (void)Binary;
        return Code;
    }

    xOptional<std::string> BuildExpressionLine(xel::xStreamReader & Reader, ssize_t & RemainSize)
    {
        assert(RemainSize > 0);
        RemainSize -= 1;

        ostringstream ss;

        auto OpStack = std::vector<std::string>();
        OpStack.reserve(2048);

        do {
            uint8_t OpCode = Reader.R1();

            (void)OpCode;
        } while(OpStack.size());

        return { ss.str() };
    }

}