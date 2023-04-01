#pragma once
#include "./_.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "../syntax/JD_JavaPrimitiveTypes.hpp"
#include "../syntax/JD_JavaObjectTypes.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <memory>

namespace jdc
{
    class xJavaMethod;

    struct xJavaLocalVariable
    {
        std::string TypeCodeName;
        std::string VariableName;
    };

    struct xJavaLocalVariableSet
    {
        std::vector<xJavaLocalVariable> VariableList;

    };

    class xJavaControlFlowGraph
    {
    public:
        xJavaLocalVariableSet LocalVariableSet;


    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaMethod * JavaMethodPtr);
    };

}
