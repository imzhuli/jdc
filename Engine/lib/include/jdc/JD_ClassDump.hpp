#pragma once
#include "./JD_Class.hpp"

namespace jdc
{

    X_GAME_API std::string DumpConstantItemString(const std::vector<xConstantItemInfo> & Items, size_t Index);
    X_GAME_API std::string DumpStringFromClass(const xClass & JavaClass);

}