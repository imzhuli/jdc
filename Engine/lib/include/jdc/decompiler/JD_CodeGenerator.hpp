#pragma once
#include "./JD_JavaSpace.hpp"
#include <filesystem>

namespace jdc
{

    X_GAME_API bool ResetClassSource(const std::filesystem::path & RootDir, const xJavaSpace * JavaSpacePtr, const xJavaClass * ClassPtr);
    X_GAME_API bool BuildClassSource(const std::filesystem::path & RootDir, const xJavaSpace * JavaSpacePtr, const xJavaClass * ClassPtr);

}