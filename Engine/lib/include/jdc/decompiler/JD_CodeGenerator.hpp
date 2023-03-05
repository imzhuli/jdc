#pragma once
#include "./JD_JavaSpace.hpp"
#include <filesystem>
#include <string>

namespace jdc
{

    X_GAME_API bool ResetClassSource(const std::filesystem::path & RootDir, const xJavaClass * ClassPtr);
    X_GAME_API bool BuildClassSource(const std::filesystem::path & RootDir, const xJavaClass * ClassPtr);
    X_GAME_API bool BuildSource(const std::string & OutputDir, const std::string & InputDir);

}