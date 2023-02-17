#include <jdc/JD_Instructions.hpp>
#include <xel/String.hpp>
#include <sstream>
#include <iostream>

using namespace xel;
using namespace std;

namespace jdc
{

    std::string BuildCode(const xMethodEx & Method, const std::vector<xConstantItemInfo> & ConstantPool)
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

        cout << "---------> Method: " << endl;
        for (auto & Variable : LocalVariables) {
            cout << "  Local: " << Variable.Name << " " << Variable.TypeString << endl;
        }
        cout << HexShow(Binary.data(), Binary.size()) << endl;

        ostringstream ss;
        auto EntryMarks = BuildEntryMarks(Binary);
        auto CodeLine = BuildExpressionLines(Binary, ConstantPool, LocalVariables, EntryMarks);

        // debug output
        ss << Method.TypeString;
        for (auto & Variable : LocalVariables) {
            cout << "  FixedLocal: " << Variable.Name << " " << Variable.TypeString << endl;
        }

        if (CodeLine()) {
            ss << " { " << endl;
            for (auto & Variable : LocalVariables) {
                if (Variable.TypeString.size() && IsLocalVariableName(Variable.Name)) {
                    ss << Variable.TypeString << ' ' << Variable.Name << ';' << endl;
                }
            }
            ss << *CodeLine << '}' << endl;
        }
        else {
            ss << ';' << endl;
        }

        auto s = ss.str();
        cout << s << endl;
        return s;
    }


}