#pragma once
#include "./_.hpp"
#include "./JD_JavaSpace.hpp"

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
        xJdcConfig                    _Config;
        std::unique_ptr<xJavaSpace>   _JavaSpace;
    };

}

