#pragma once
#include "./JD_Class.hpp"
#include "./JD_ClassEx.hpp"

namespace jdc
{

    X_GAME_API std::string DumpConstantItemString(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API std::string DumpAttribute(const std::vector<xConstantItemInfo> & ConstantPool, const xAttributeInfo & AttributeInfo);
    X_GAME_API std::string DumpFieldDescriptor(const std::string & Utf8);
    X_GAME_API std::string DumpMethodDescriptor(const std::string & MethodName, const xMethodDescriptor & Descriptor);
    X_GAME_API std::string Dump(const xClass & JavaClass);

    X_GAME_API std::string DumpFieldAccessFlags(xAccessFlag Flags);
    X_GAME_API std::string DumpClassAccessFlags(const xClass & JavaClass);

    X_GAME_API std::string Dump(const xFieldEx & FieldEx);

}