#pragma once
#include "./JD_Class.hpp"
#include <string>
#include <utility>

namespace jdc
{

    X_GAME_API std::vector<std::string> GetImportNames(const xClassInfo & JavaClass);
    X_GAME_API std::vector<std::string> GetInterfaceNames(const xClassInfo & JavaClass);
    X_GAME_API std::string GenerateClassTitle(const xClassInfo & JavaClass);

    X_GAME_API std::string GenerateClassCode(const xClassInfo & JavaClass);

}