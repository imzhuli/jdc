#pragma once
#include "./_.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "../syntax/JD_JavaPrimitiveTypes.hpp"
#include "../syntax/JD_JavaObjectTypes.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <memory>

namespace jdc
{
    class xJavaClass;


    class xLocalVariableSet
    {

    };

    class xJavaControlFlowGraph
    {
    public:
        xLocalVariableSet LocalVariableSet;


    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaClass * JavaClassPtr, const xAttributeCode & CodeAttribute);
    };

}
