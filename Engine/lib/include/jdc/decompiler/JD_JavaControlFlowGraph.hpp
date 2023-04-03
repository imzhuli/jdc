#pragma once
#include "./_.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "../syntax/JD_JavaPrimitiveTypes.hpp"
#include "../syntax/JD_JavaObjectTypes.hpp"
#include "../syntax/JD_JavaFrame.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <memory>

namespace jdc
{
    class xJavaMethod;

    class xJavaControlFlowGraph
    {
    public:
        std::vector<xJavaLocalVariable> LocalVariableList;

    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaMethod * JavaMethodPtr);
    };

}
