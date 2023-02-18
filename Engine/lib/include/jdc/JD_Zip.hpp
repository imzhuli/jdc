#pragma once
#include "./JD_Base.hpp"

namespace jdc
{

    X_GAME_API bool UnzipJar(const std::string & TargetPath, const std::string & ZipFilename);
    X_GAME_API bool ZipJar(const std::string & TargetZipFilename, const std::string & SourceClassPath);

}