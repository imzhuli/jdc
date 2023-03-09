#pragma once
#include "./base/_.hpp"
#include "./decompiler/_.hpp"

namespace jdc
{

    using xJdcHandle = struct xJdc *;

    X_GAME_API xJdcHandle InitJdc(const xJdcConfig & Config);
    X_GAME_API bool ExecuteJdc(xJdcHandle JdcHandle);
    X_GAME_API void CleanJdc(xJdcHandle JdcHandle);

}
