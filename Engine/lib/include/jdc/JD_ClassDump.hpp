#pragma once
#include "./JD_Class.hpp"

namespace jdc
{

    X_GAME_API std::string DumpConstantItemString(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API std::string DumpAttribute(const std::vector<xConstantItemInfo> & ConstantPool, const xAttributeInfo & AttributeInfo);
    X_GAME_API std::string DumpClass(const xClass & JavaClass);

}