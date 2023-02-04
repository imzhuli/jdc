#pragma once
#include "./JD_Class.hpp"
#include <string>
#include <utility>

namespace jdc
{

    X_GAME_API std::vector<std::string> GetImportNames(const xClass & JavaClass);
    X_GAME_API std::vector<std::string> GetInterfaceNames(const xClass & JavaClass);
    X_GAME_API std::string GenerateClassTitle(const xClass & JavaClass);

    X_GAME_API std::string GenerateClassCode(const xClass & JavaClass);

}