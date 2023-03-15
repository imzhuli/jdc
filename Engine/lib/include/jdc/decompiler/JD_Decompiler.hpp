#pragma once
#include "./_.hpp"
#include "./JD_JavaSpace.hpp"
#include <filesystem>

namespace jdc
{

    class xJdc final
    : xel::xNonCopyable
    {
    public:

        X_PRIVATE_MEMBER bool Init(const xJdcConfig & Config);
        X_PRIVATE_MEMBER bool Execute();
        X_PRIVATE_MEMBER void Clean();

        const xJdcConfig & GetConfig() const { return _Config; }

    private:
        X_PRIVATE_MEMBER bool MakePackagePaths();

    private:
        xJdcConfig                    _Config;
        std::unique_ptr<xJavaSpace>   _JavaSpaceUPtr;
        std::filesystem::path         _OutputRootDirectory;
    };

}

